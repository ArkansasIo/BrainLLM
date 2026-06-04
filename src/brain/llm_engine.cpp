#include "llm_engine.h"
#include <QDir>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QProcess>
#include <algorithm>
#include <cctype>
#include <cmath>
#include <map>
#include <numeric>
#include <sstream>
#include <set>

namespace BrainLLM {

namespace {

std::string to_lower(std::string value) {
    std::transform(value.begin(), value.end(), value.begin(),
                   [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return value;
}

std::vector<std::string> words_from(const std::string& text) {
    std::vector<std::string> words;
    std::string current;
    for (unsigned char c : text) {
        if (std::isalnum(c)) {
            current += static_cast<char>(std::tolower(c));
        } else if (!current.empty()) {
            words.push_back(current);
            current.clear();
        }
    }
    if (!current.empty()) {
        words.push_back(current);
    }
    return words;
}

std::vector<std::string> sentences_from(const std::string& text) {
    std::vector<std::string> sentences;
    std::string current;
    for (char c : text) {
        current += c;
        if (c == '.' || c == '!' || c == '?') {
            current.erase(current.begin(), std::find_if(current.begin(), current.end(), [](unsigned char ch) {
                return !std::isspace(ch);
            }));
            current.erase(std::find_if(current.rbegin(), current.rend(), [](unsigned char ch) {
                return !std::isspace(ch);
            }).base(), current.end());
            if (!current.empty()) {
                sentences.push_back(current);
            }
            current.clear();
        }
    }
    current.erase(current.begin(), std::find_if(current.begin(), current.end(), [](unsigned char ch) {
        return !std::isspace(ch);
    }));
    current.erase(std::find_if(current.rbegin(), current.rend(), [](unsigned char ch) {
        return !std::isspace(ch);
    }).base(), current.end());
    if (!current.empty()) {
        sentences.push_back(current);
    }
    return sentences;
}

bool contains_any(const std::set<std::string>& words, const std::vector<std::string>& candidates) {
    for (const auto& candidate : candidates) {
        if (words.find(candidate) != words.end()) {
            return true;
        }
    }
    return false;
}

float token_overlap_score(const std::vector<std::string>& left, const std::vector<std::string>& right) {
    if (left.empty() || right.empty()) {
        return 0.0f;
    }

    std::set<std::string> left_set(left.begin(), left.end());
    std::set<std::string> right_set(right.begin(), right.end());
    size_t overlap = 0;
    for (const auto& token : left_set) {
        if (right_set.find(token) != right_set.end()) {
            ++overlap;
        }
    }
    return static_cast<float>(overlap) /
           static_cast<float>(std::max<size_t>(1, std::min(left_set.size(), right_set.size())));
}

std::string first_sentence(const std::string& text) {
    auto end = text.find_first_of(".!?");
    if (end == std::string::npos) {
        return text;
    }
    return text.substr(0, end + 1);
}

std::string join_keywords(const std::vector<std::string>& words, size_t max_count = 5) {
    static const std::set<std::string> stop_words = {
        "the", "a", "an", "and", "or", "but", "to", "of", "in", "on", "for", "with",
        "is", "are", "was", "were", "be", "been", "it", "this", "that", "i", "you"
    };

    std::vector<std::string> selected;
    for (const auto& word : words) {
        if (word.size() > 2 && stop_words.find(word) == stop_words.end() &&
            std::find(selected.begin(), selected.end(), word) == selected.end()) {
            selected.push_back(word);
        }
        if (selected.size() >= max_count) {
            break;
        }
    }

    if (selected.empty()) {
        return "general context";
    }

    std::ostringstream oss;
    for (size_t i = 0; i < selected.size(); ++i) {
        if (i > 0) {
            oss << ", ";
        }
        oss << selected[i];
    }
    return oss.str();
}

std::string state_name(BrainState state) {
    switch (state) {
        case BrainState::Idle: return "idle";
        case BrainState::Processing: return "processing";
        case BrainState::Learning: return "learning";
        case BrainState::Sleeping: return "sleeping";
        case BrainState::Error: return "error";
    }
    return "unknown";
}

bool looks_computational_query(const std::string& prompt, const std::set<std::string>& words) {
    if (prompt.find("wolfram:") == 0 || prompt.find("Wolfram:") == 0) {
        return true;
    }

    if (prompt.find_first_of("=+-*/^") != std::string::npos) {
        return true;
    }

    return contains_any(words, {
        "calculate", "compute", "solve", "integrate", "derive", "derivative", "equation",
        "convert", "distance", "population", "weather", "statistics", "plot", "factor",
        "simplify", "unit", "units", "constant", "chemistry", "physics", "astronomy"
    });
}

std::string strip_tool_prefix(const std::string& prompt) {
    const std::string lower = to_lower(prompt);
    const std::string prefix = "wolfram:";
    if (lower.find(prefix) == 0) {
        size_t start = prefix.size();
        while (start < prompt.size() && std::isspace(static_cast<unsigned char>(prompt[start]))) {
            ++start;
        }
        return prompt.substr(start);
    }
    return prompt;
}

} // namespace

LLMEngine::LLMEngine(const BrainConfig& config)
    : config_(config), state_(BrainState::Idle), confidence_(0.0f) {
    neural_net_ = std::make_unique<NeuralNetwork>(config);
    memory_ = std::make_unique<MemorySystem>(config.max_memory_size);
    attention_ = std::make_unique<AttentionMechanism>(config.num_attention_heads, config.embedding_dim);
    wolfram_alpha_ = std::make_unique<WolframAlphaClient>();
    // AirLLM bridge is default-constructed; call configure_airllm() to activate it.
}

std::string LLMEngine::process_input(const std::string& input) {
    state_ = BrainState::Processing;

    context_.current_input = input;
    auto words = words_from(input);
    std::set<std::string> word_set(words.begin(), words.end());
    auto memories = memory_->retrieve_memories(input, 3);

    std::string response;
    if (input.empty()) {
        response = "I need input text before I can process anything.";
        confidence_ = 0.25f;
    } else if (contains_any(word_set, {"status", "metrics", "health"})) {
        auto metrics = get_metrics();
        std::ostringstream oss;
        oss << "BrainLLM is " << state_name(state_)
            << ". Accuracy estimate: " << metrics.accuracy
            << ", tokens processed: " << metrics.tokens_processed
            << ", confidence: " << confidence_ << ".";
        response = oss.str();
        confidence_ = 0.82f;
    } else if (contains_any(word_set, {"remember", "memorize", "save"})) {
        memory_->store_memory(input, 0.9f);
        response = "Stored this as a high-importance memory: " + first_sentence(input);
        confidence_ = 0.88f;
    } else if (contains_any(word_set, {"summarize", "summary"})) {
        response = "Summary: " + first_sentence(input) + " Key topics: " + join_keywords(words) + ".";
        confidence_ = 0.78f;
    } else if (contains_any(word_set, {"explain", "why", "how", "what"})) {
        response = "Explanation: the main topic appears to be " + join_keywords(words) +
                   ". A practical next step is to break it into inputs, constraints, expected output, and verification.";
        confidence_ = 0.74f;
    } else if (!memories.empty() && memories[0].importance > 0.0f) {
        response = "I connected this with memory: " + memories[0].content +
                   ". Current interpretation: " + first_sentence(input);
        confidence_ = 0.72f;
    } else {
        response = "Processed input. Key topics: " + join_keywords(words) +
                   ". I can summarize, explain, remember, train, or generate a response from this context.";
        confidence_ = 0.68f;
    }

    context_.last_output = response;
    store_interaction(input, response);

    state_ = BrainState::Idle;
    return response;
}

std::string LLMEngine::generate_response(const std::string& prompt, int max_tokens) {
    state_ = BrainState::Processing;

    const PromptAnalysis analysis = analyze_prompt(prompt);
    auto words = analysis.tokens;
    std::set<std::string> word_set(words.begin(), words.end());
    auto memories = memory_->retrieve_memories(prompt, 3);

    std::ostringstream response;
    response << "BrainLLM response: ";

    if (prompt.empty()) {
        response << "provide a prompt and I will generate a response.";
        confidence_ = 0.25f;
    } else if (analysis.intent == LLMIntent::Compute) {
        // --- Wolfram Alpha: mathematics, science, unit conversion, factual queries ---
        const std::string query = strip_tool_prefix(prompt);
        const std::string wolfram_answer = query_wolfram_alpha(query, true);
        response << wolfram_answer;
        confidence_ = wolfram_answer.find("not configured") == std::string::npos ? 0.86f : 0.55f;
    } else if (analysis.intent == LLMIntent::Summarize ||
               analysis.intent == LLMIntent::Explain ||
               analysis.intent == LLMIntent::Plan ||
               analysis.intent == LLMIntent::Remember ||
               analysis.intent == LLMIntent::Recall ||
               analysis.intent == LLMIntent::Train ||
               analysis.intent == LLMIntent::Status ||
               analysis.intent == LLMIntent::Creative ||
               !training_examples_.empty() ||
               !next_token_counts_.empty()) {
        response.str("");
        response << synthesize_local_response(analysis, memories, max_tokens);
    } else if (airllm_bridge_.is_source_available() && !airllm_bridge_.get_config().model_id.empty()) {
        // --- AirLLM: use the configured language model for natural-language generation ---
        const std::string airllm_result = run_airllm_inference(prompt);
        if (airllm_result.find("AirLLM error:") != 0) {
            response.str("");  // clear "BrainLLM response: " prefix
            response << airllm_result;
            confidence_ = 0.88f;
        } else {
            // AirLLM failed; fall through to heuristic response
            response << "I understand the prompt as focused on " << join_keywords(words) << ". ";
            if (!memories.empty()) {
                response << "Relevant memory: " << first_sentence(memories[0].content) << " ";
            }
            response << "[AirLLM unavailable: " << airllm_result << "]";
            confidence_ = 0.55f;
        }
    } else {
        // --- Heuristic fallback when neither Wolfram nor AirLLM is active ---
        response.str("");
        response << synthesize_local_response(analysis, memories, max_tokens);
    }

    std::string output = response.str();
    if (max_tokens > 0) {
        const size_t approx_chars = static_cast<size_t>(max_tokens) * 5;
        if (output.size() > approx_chars) {
            output = output.substr(0, approx_chars);
        }
    }

    context_.current_input = prompt;
    context_.last_output = output;
    store_interaction(prompt, output);

    state_ = BrainState::Idle;
    return output;
}

std::string LLMEngine::query_wolfram_alpha(const std::string& query, bool llm_format) {
    if (!wolfram_alpha_) {
        wolfram_alpha_ = std::make_unique<WolframAlphaClient>();
    }

    const QString q = QString::fromStdString(query).trimmed();
    const WolframAlphaResult result = llm_format
        ? wolfram_alpha_->llm_answer(q)
        : wolfram_alpha_->short_answer(q);

    std::ostringstream oss;
    if (result.success) {
        oss << "Wolfram Alpha result: " << result.answer.toStdString();
        if (!result.source_url.isEmpty()) {
            oss << " Source: " << result.source_url.toStdString();
        }
    } else {
        oss << "Wolfram Alpha is available as an external computation tool, but " 
            << result.error.toStdString();
        if (!result.source_url.isEmpty()) {
            oss << " Manual query: " << result.source_url.toStdString();
        }
    }

    return oss.str();
}

void LLMEngine::train(const std::vector<std::string>& training_data) {
    state_ = BrainState::Learning;

    for (const auto& data : training_data) {
        if (data.empty()) {
            continue;
        }
        auto encoded = encode_input(data);
        neural_net_->forward(encoded);
        neural_net_->backward(encoded);
        neural_net_->update_weights(config_.learning_rate);
        memory_->store_memory("Training sample: " + data, 0.85f);
        learn_sequence_model(data);

        const size_t arrow = data.find("->");
        const size_t fat_arrow = data.find("=>");
        const size_t split = arrow != std::string::npos ? arrow : fat_arrow;
        if (split != std::string::npos) {
            TrainingExample example;
            example.input = data.substr(0, split);
            example.output = data.substr(split + 2);
            example.input_tokens = words_from(example.input);
            training_examples_.push_back(example);
        }
    }
    confidence_ = std::min(1.0f, confidence_ + static_cast<float>(training_data.size()) * 0.01f);

    state_ = BrainState::Idle;
}

void LLMEngine::update(const std::string& input, const std::string& expected_output) {
    auto encoded_input = encode_input(input);
    auto encoded_expected = encode_input(expected_output);
    
    neural_net_->forward(encoded_input);
    neural_net_->backward(encoded_expected);
    neural_net_->update_weights(config_.learning_rate);
    
    memory_->store_memory(input + " -> " + expected_output, 0.9f);
    TrainingExample example;
    example.input = input;
    example.output = expected_output;
    example.input_tokens = words_from(input);
    training_examples_.push_back(example);
    learn_sequence_model(input + " " + expected_output);
}

void LLMEngine::initialize() {
    state_ = BrainState::Processing;
    neural_net_->initialize_weights();
    memory_->clear_memories();
    state_ = BrainState::Idle;
}

void LLMEngine::reset() {
    state_ = BrainState::Idle;
    neural_net_->reset();
    memory_->clear_memories();
    context_ = LanguageContext();
    confidence_ = 0.0f;
    training_examples_.clear();
    next_token_counts_.clear();
}

BrainState LLMEngine::get_state() const {
    return state_;
}

void LLMEngine::set_state(BrainState state) {
    state_ = state;
}

BrainMetrics LLMEngine::get_metrics() const {
    return neural_net_->get_metrics();
}

float LLMEngine::get_confidence() const {
    return confidence_;
}

void LLMEngine::update_config(const BrainConfig& config) {
    config_ = config;
}

BrainConfig LLMEngine::get_config() const {
    return config_;
}

LanguageContext LLMEngine::get_context() const {
    return context_;
}

void LLMEngine::set_context(const LanguageContext& context) {
    context_ = context;
}

std::vector<MemoryRecord> LLMEngine::recall_memories(const std::string& query) {
    return memory_->retrieve_memories(query);
}

void LLMEngine::store_interaction(const std::string& input, const std::string& output) {
    memory_->store_memory(input + " | " + output, confidence_);
}

std::vector<float> LLMEngine::tokenize(const std::string& text) {
    std::vector<float> tokens;
    auto words = words_from(text);
    for (const auto& word : words) {
        uint32_t hash = 2166136261u;
        for (unsigned char c : word) {
            hash ^= c;
            hash *= 16777619u;
        }
        tokens.push_back(static_cast<float>(hash % config_.vocab_size) /
                         static_cast<float>(std::max(1, config_.vocab_size)));
    }
    return tokens;
}

std::string LLMEngine::detokenize(const std::vector<float>& tokens) {
    std::string text;
    for (float token : tokens) {
        text += char(static_cast<int>(token * 256.0f));
    }
    return text;
}

Activation LLMEngine::encode_input(const std::string& input) {
    Activation activation(config_.embedding_dim, 0.0f);
    auto tokens = tokenize(input);
    
    for (size_t i = 0; i < std::min(tokens.size(), activation.size()); ++i) {
        activation[i] = tokens[i];
    }
    
    return activation;
}

std::string LLMEngine::decode_output(const Activation& output) {
    if (output.empty()) {
        return "";
    }

    float average = 0.0f;
    for (float value : output) {
        average += value;
    }
    average /= static_cast<float>(output.size());

    std::ostringstream oss;
    oss << "Neural activation average: " << average;
    return oss.str();
}

LLMEngine::PromptAnalysis LLMEngine::analyze_prompt(const std::string& prompt) const {
    PromptAnalysis analysis;
    analysis.original = prompt;
    analysis.tokens = words_from(prompt);
    analysis.sentences = sentences_from(prompt);
    analysis.asks_question = prompt.find('?') != std::string::npos;
    analysis.embedding = embed_text(prompt);

    std::set<std::string> word_set(analysis.tokens.begin(), analysis.tokens.end());
    if (looks_computational_query(prompt, word_set)) {
        analysis.intent = LLMIntent::Compute;
    } else if (contains_any(word_set, {"status", "metrics", "health"})) {
        analysis.intent = LLMIntent::Status;
    } else if (contains_any(word_set, {"summarize", "summary", "brief", "condense", "tldr"})) {
        analysis.intent = LLMIntent::Summarize;
    } else if (contains_any(word_set, {"explain", "why", "how", "what", "describe"})) {
        analysis.intent = LLMIntent::Explain;
    } else if (contains_any(word_set, {"plan", "steps", "roadmap", "todo", "build", "create", "implement"})) {
        analysis.intent = LLMIntent::Plan;
    } else if (contains_any(word_set, {"remember", "memorize", "save"})) {
        analysis.intent = LLMIntent::Remember;
    } else if (contains_any(word_set, {"recall", "memory", "stored"})) {
        analysis.intent = LLMIntent::Recall;
    } else if (contains_any(word_set, {"train", "learn", "teach", "update"})) {
        analysis.intent = LLMIntent::Train;
    } else if (contains_any(word_set, {"write", "story", "poem", "draft", "creative"})) {
        analysis.intent = LLMIntent::Creative;
    }

    static const std::set<std::string> stop_words = {
        "the", "a", "an", "and", "or", "but", "to", "of", "in", "on", "for", "with",
        "is", "are", "was", "were", "be", "been", "it", "this", "that", "i", "you",
        "me", "my", "we", "our", "they", "them", "as", "at", "by", "from"
    };

    std::map<std::string, int> frequencies;
    for (const auto& token : analysis.tokens) {
        if (token.size() > 2 && stop_words.find(token) == stop_words.end()) {
            frequencies[token]++;
        }
    }
    std::vector<std::pair<std::string, int>> ranked(frequencies.begin(), frequencies.end());
    std::sort(ranked.begin(), ranked.end(), [](const auto& a, const auto& b) {
        if (a.second != b.second) return a.second > b.second;
        return a.first < b.first;
    });
    for (const auto& item : ranked) {
        analysis.keywords.push_back(item.first);
        if (analysis.keywords.size() >= 8) {
            break;
        }
    }

    const float sentence_factor = static_cast<float>(analysis.sentences.size()) * 0.08f;
    const float token_factor = static_cast<float>(analysis.tokens.size()) / 80.0f;
    analysis.complexity = std::min(1.0f, sentence_factor + token_factor);
    analysis.contains_risky_pattern = contains_any(word_set, {
        "ignore", "override", "jailbreak", "secret", "password", "token", "credential"
    });
    analysis.compressed_context = summarize_text(analysis, 2);
    return analysis;
}

std::string LLMEngine::summarize_text(const PromptAnalysis& analysis, int max_sentences) const {
    if (analysis.sentences.empty()) {
        return "Summary: " + first_sentence(analysis.original);
    }

    std::vector<std::pair<float, std::string>> scored;
    for (const auto& sentence : analysis.sentences) {
        const auto sentence_words = words_from(sentence);
        float score = token_overlap_score(sentence_words, analysis.keywords);
        score += std::min(0.25f, static_cast<float>(sentence_words.size()) / 120.0f);
        scored.emplace_back(score, sentence);
    }
    std::sort(scored.begin(), scored.end(), [](const auto& a, const auto& b) {
        return a.first > b.first;
    });

    std::ostringstream oss;
    oss << "Summary: ";
    const int count = std::min(max_sentences, static_cast<int>(scored.size()));
    for (int i = 0; i < count; ++i) {
        if (i > 0) oss << " ";
        oss << scored[i].second;
    }
    oss << " Key topics: " << join_keywords(analysis.keywords.empty() ? analysis.tokens : analysis.keywords) << ".";
    return oss.str();
}

std::string LLMEngine::explain_text(const PromptAnalysis& analysis,
                                    const std::vector<MemoryRecord>& memories) const {
    const auto ranked_memories = rerank_memories_semantically(analysis, memories);
    std::ostringstream oss;
    oss << "Explanation: the prompt centers on "
        << join_keywords(analysis.keywords.empty() ? analysis.tokens : analysis.keywords)
        << ". ";
    if (!ranked_memories.empty()) {
        oss << "Memory context suggests: " << first_sentence(ranked_memories.front().content) << " ";
    }
    oss << "Break it down into intent, inputs, constraints, mechanism, output, and verification. ";
    oss << "Complexity estimate: " << analysis.complexity << ".";
    return oss.str();
}

std::string LLMEngine::plan_from_prompt(const PromptAnalysis& analysis) const {
    const std::string topic = join_keywords(analysis.keywords.empty() ? analysis.tokens : analysis.keywords);
    std::ostringstream oss;
    oss << "Plan for " << topic << ": ";
    oss << "1. Define the target behavior. ";
    oss << "2. Inspect existing inputs, outputs, and constraints. ";
    oss << "3. Implement the smallest complete path. ";
    oss << "4. Add memory, routing, and error handling where needed. ";
    oss << "5. Build and verify the UI/API path.";
    return oss.str();
}

std::string LLMEngine::retrieve_trained_response(const PromptAnalysis& analysis, float* score) const {
    float best_score = 0.0f;
    std::string best_output;
    for (const auto& example : training_examples_) {
        const float current_score = token_overlap_score(analysis.tokens, example.input_tokens);
        if (current_score > best_score) {
            best_score = current_score;
            best_output = example.output;
        }
    }
    if (score) {
        *score = best_score;
    }
    return best_output;
}

std::string LLMEngine::generate_from_markov(const PromptAnalysis& analysis, int max_tokens) const {
    if (next_token_counts_.empty()) {
        return "Draft: " + first_sentence(analysis.original) +
               " This can be expanded with clearer context, stronger structure, and a verified ending.";
    }

    std::string current = analysis.tokens.empty() ? "<START>" : analysis.tokens.back();
    std::vector<std::string> generated;
    const int limit = std::max(8, std::min(max_tokens, 80));
    for (int i = 0; i < limit; ++i) {
        auto it = next_token_counts_.find(current);
        if (it == next_token_counts_.end() || it->second.empty()) {
            it = next_token_counts_.find("<START>");
            if (it == next_token_counts_.end() || it->second.empty()) {
                break;
            }
        }
        const auto best = std::max_element(it->second.begin(), it->second.end(), [](const auto& a, const auto& b) {
            if (a.second != b.second) return a.second < b.second;
            return a.first > b.first;
        });
        if (best == it->second.end() || best->first == "<END>") {
            break;
        }
        generated.push_back(best->first);
        current = best->first;
    }

    if (generated.empty()) {
        return "Draft: " + first_sentence(analysis.original);
    }

    std::ostringstream oss;
    oss << "Draft:";
    for (const auto& token : generated) {
        oss << " " << token;
    }
    oss << ".";
    return oss.str();
}

std::vector<std::pair<std::string, float>> LLMEngine::top_next_tokens(const std::string& token,
                                                                      int top_k) const {
    std::vector<std::pair<std::string, float>> ranked;
    auto it = next_token_counts_.find(token);
    if (it == next_token_counts_.end()) {
        it = next_token_counts_.find("<START>");
    }
    if (it == next_token_counts_.end() || it->second.empty()) {
        return ranked;
    }

    int total = 0;
    for (const auto& item : it->second) {
        total += item.second;
    }
    for (const auto& item : it->second) {
        ranked.emplace_back(item.first, static_cast<float>(item.second) /
                                        static_cast<float>(std::max(1, total)));
    }
    std::sort(ranked.begin(), ranked.end(), [](const auto& a, const auto& b) {
        if (a.second != b.second) return a.second > b.second;
        return a.first < b.first;
    });
    if (ranked.size() > static_cast<size_t>(top_k)) {
        ranked.resize(top_k);
    }
    return ranked;
}

std::string LLMEngine::generate_with_beam_search(const PromptAnalysis& analysis,
                                                 int max_tokens,
                                                 int beam_width) const {
    if (next_token_counts_.empty()) {
        return generate_from_markov(analysis, max_tokens);
    }

    struct Beam {
        std::vector<std::string> tokens;
        std::string current;
        float score;
        bool finished;
    };

    std::vector<Beam> beams = {{
        {},
        analysis.tokens.empty() ? std::string("<START>") : analysis.tokens.back(),
        1.0f,
        false
    }};

    const int limit = std::max(8, std::min(max_tokens, 80));
    for (int step = 0; step < limit; ++step) {
        std::vector<Beam> candidates;
        for (const auto& beam : beams) {
            if (beam.finished) {
                candidates.push_back(beam);
                continue;
            }
            const auto next_tokens = top_next_tokens(beam.current, beam_width);
            if (next_tokens.empty()) {
                Beam finished = beam;
                finished.finished = true;
                candidates.push_back(finished);
                continue;
            }
            for (const auto& [next, probability] : next_tokens) {
                Beam expanded = beam;
                expanded.current = next;
                expanded.score *= std::max(0.0001f, probability);
                if (next == "<END>") {
                    expanded.finished = true;
                } else {
                    expanded.tokens.push_back(next);
                }
                candidates.push_back(expanded);
            }
        }
        std::sort(candidates.begin(), candidates.end(), [](const Beam& a, const Beam& b) {
            const float a_norm = a.score / static_cast<float>(std::max<size_t>(1, a.tokens.size()));
            const float b_norm = b.score / static_cast<float>(std::max<size_t>(1, b.tokens.size()));
            return a_norm > b_norm;
        });
        if (candidates.size() > static_cast<size_t>(beam_width)) {
            candidates.resize(beam_width);
        }
        beams = candidates;
        if (std::all_of(beams.begin(), beams.end(), [](const Beam& beam) { return beam.finished; })) {
            break;
        }
    }

    if (beams.empty() || beams.front().tokens.empty()) {
        return generate_from_markov(analysis, max_tokens);
    }

    std::ostringstream oss;
    oss << "Draft:";
    for (const auto& token : beams.front().tokens) {
        oss << " " << token;
    }
    oss << ".";
    return oss.str();
}

std::vector<float> LLMEngine::embed_text(const std::string& text, int dimensions) const {
    std::vector<float> embedding(std::max(1, dimensions), 0.0f);
    const auto tokens = words_from(text);
    for (const auto& token : tokens) {
        uint32_t hash = 2166136261u;
        for (unsigned char c : token) {
            hash ^= c;
            hash *= 16777619u;
        }
        const size_t index = hash % embedding.size();
        const float sign = (hash & 1u) ? 1.0f : -1.0f;
        embedding[index] += sign * (1.0f + std::min(8.0f, static_cast<float>(token.size())) / 8.0f);
    }
    const float norm = std::sqrt(std::inner_product(embedding.begin(), embedding.end(), embedding.begin(), 0.0f));
    if (norm > 0.0f) {
        for (float& value : embedding) {
            value /= norm;
        }
    }
    return embedding;
}

float LLMEngine::cosine_similarity(const std::vector<float>& a, const std::vector<float>& b) const {
    if (a.empty() || b.empty()) {
        return 0.0f;
    }
    const size_t size = std::min(a.size(), b.size());
    float dot = 0.0f;
    float norm_a = 0.0f;
    float norm_b = 0.0f;
    for (size_t i = 0; i < size; ++i) {
        dot += a[i] * b[i];
        norm_a += a[i] * a[i];
        norm_b += b[i] * b[i];
    }
    if (norm_a <= 0.0f || norm_b <= 0.0f) {
        return 0.0f;
    }
    return dot / (std::sqrt(norm_a) * std::sqrt(norm_b));
}

std::vector<MemoryRecord> LLMEngine::rerank_memories_semantically(
    const PromptAnalysis& analysis,
    const std::vector<MemoryRecord>& memories) const {
    std::vector<std::pair<float, MemoryRecord>> scored;
    for (const auto& memory : memories) {
        const float semantic = cosine_similarity(analysis.embedding, embed_text(memory.content));
        const float lexical = token_overlap_score(analysis.tokens, words_from(memory.content));
        const float score = semantic * 0.55f + lexical * 0.30f + memory.importance * 0.15f;
        scored.emplace_back(score, memory);
    }
    std::sort(scored.begin(), scored.end(), [](const auto& a, const auto& b) {
        return a.first > b.first;
    });

    std::vector<MemoryRecord> ranked;
    for (const auto& item : scored) {
        ranked.push_back(item.second);
    }
    return ranked;
}

std::string LLMEngine::compress_prompt_context(const PromptAnalysis& analysis,
                                               const std::vector<MemoryRecord>& memories,
                                               size_t max_chars) const {
    std::ostringstream context;
    context << "Intent: ";
    switch (analysis.intent) {
        case LLMIntent::Summarize: context << "summarize"; break;
        case LLMIntent::Explain: context << "explain"; break;
        case LLMIntent::Plan: context << "plan"; break;
        case LLMIntent::Remember: context << "remember"; break;
        case LLMIntent::Recall: context << "recall"; break;
        case LLMIntent::Train: context << "train"; break;
        case LLMIntent::Compute: context << "compute"; break;
        case LLMIntent::Status: context << "status"; break;
        case LLMIntent::Creative: context << "creative"; break;
        case LLMIntent::Chat: default: context << "chat"; break;
    }
    context << ". Topics: " << join_keywords(analysis.keywords.empty() ? analysis.tokens : analysis.keywords) << ". ";
    const auto ranked = rerank_memories_semantically(analysis, memories);
    if (!ranked.empty()) {
        context << "Memory: " << first_sentence(ranked.front().content) << " ";
    }
    context << "Prompt: " << first_sentence(analysis.original);
    std::string value = context.str();
    if (value.size() > max_chars) {
        value = value.substr(0, max_chars);
    }
    return value;
}

std::string LLMEngine::validate_and_repair_response(const PromptAnalysis& analysis,
                                                    const std::string& response) const {
    std::string repaired = response;
    if (repaired.empty()) {
        repaired = "I need more context before I can produce a useful answer.";
    }
    if (analysis.contains_risky_pattern) {
        repaired += " I will keep the response focused on safe, visible project behavior and avoid secrets or credential handling.";
    }
    if (!repaired.empty() && repaired.back() != '.' && repaired.back() != '!' && repaired.back() != '?' && repaired.back() != '`') {
        repaired += ".";
    }
    const size_t repeated = repaired.find("  ");
    if (repeated != std::string::npos) {
        std::string compact;
        bool previous_space = false;
        for (char c : repaired) {
            if (std::isspace(static_cast<unsigned char>(c))) {
                if (!previous_space) {
                    compact += ' ';
                }
                previous_space = true;
            } else {
                compact += c;
                previous_space = false;
            }
        }
        repaired = compact;
    }
    return repaired;
}

std::string LLMEngine::synthesize_local_response(const PromptAnalysis& analysis,
                                                 const std::vector<MemoryRecord>& memories,
                                                 int max_tokens) {
    float trained_score = 0.0f;
    const std::string trained = retrieve_trained_response(analysis, &trained_score);
    if (!trained.empty() && trained_score >= 0.45f) {
        confidence_ = std::min(0.95f, 0.70f + trained_score * 0.25f);
        return trained;
    }

    std::ostringstream response;
    switch (analysis.intent) {
        case LLMIntent::Summarize:
            response << summarize_text(analysis);
            confidence_ = 0.78f;
            break;
        case LLMIntent::Explain:
            response << explain_text(analysis, memories);
            confidence_ = memories.empty() ? 0.76f : 0.84f;
            break;
        case LLMIntent::Plan:
            response << plan_from_prompt(analysis);
            confidence_ = 0.80f;
            break;
        case LLMIntent::Remember:
            memory_->store_memory(analysis.original, 0.9f);
            response << "Stored this memory with high importance. Key topics: "
                     << join_keywords(analysis.keywords.empty() ? analysis.tokens : analysis.keywords) << ".";
            confidence_ = 0.88f;
            break;
        case LLMIntent::Recall:
            if (memories.empty()) {
                response << "I do not have a strong matching memory yet.";
                confidence_ = 0.50f;
            } else {
                response << "Most relevant memory: " << first_sentence(memories.front().content);
                confidence_ = 0.78f;
            }
            break;
        case LLMIntent::Train:
            response << "Training data accepted. Use `input -> expected output` samples to teach exact response patterns.";
            confidence_ = 0.72f;
            break;
        case LLMIntent::Creative:
            response << generate_with_beam_search(analysis, max_tokens, 3);
            confidence_ = next_token_counts_.empty() ? 0.62f : 0.76f;
            break;
        case LLMIntent::Status:
            response << "BrainLLM is " << state_name(state_) << " with confidence " << confidence_
                     << " and " << training_examples_.size() << " trained examples.";
            confidence_ = 0.84f;
            break;
        case LLMIntent::Chat:
        case LLMIntent::Compute:
        default:
            response << "I read this as a request about "
                     << join_keywords(analysis.keywords.empty() ? analysis.tokens : analysis.keywords)
                     << ". ";
            if (!memories.empty()) {
                const auto ranked_memories = rerank_memories_semantically(analysis, memories);
                response << "Relevant memory: " << first_sentence(ranked_memories.front().content) << " ";
            }
            response << "Recommended answer: " << first_sentence(analysis.original);
            response << (analysis.asks_question
                ? " My best next step is to answer directly, identify assumptions, and verify the result."
                : " The useful path is to clarify the goal, act on the smallest working step, and verify it.");
            confidence_ = memories.empty() ? 0.70f : 0.80f;
            break;
    }
    const std::string compressed = compress_prompt_context(analysis, memories);
    Q_UNUSED(compressed);
    return validate_and_repair_response(analysis, response.str());
}

void LLMEngine::learn_sequence_model(const std::string& text) {
    const auto tokens = words_from(text);
    if (tokens.empty()) {
        return;
    }
    next_token_counts_["<START>"][tokens.front()]++;
    for (size_t i = 0; i + 1 < tokens.size(); ++i) {
        next_token_counts_[tokens[i]][tokens[i + 1]]++;
    }
    next_token_counts_[tokens.back()]["<END>"]++;
}

// ========================================
// AIRLLM INTEGRATION
// ========================================

void LLMEngine::configure_airllm(const AirLLMRuntimeConfig& cfg) {
    airllm_bridge_.configure(cfg);
}

AirLLMRuntimeConfig LLMEngine::get_airllm_config() const {
    return airllm_bridge_.get_config();
}

AirLLMBridge& LLMEngine::get_airllm_bridge() {
    return airllm_bridge_;
}

const AirLLMBridge& LLMEngine::get_airllm_bridge() const {
    return airllm_bridge_;
}

bool LLMEngine::is_airllm_available() const {
    return airllm_bridge_.is_source_available() &&
           !airllm_bridge_.get_config().model_id.empty();
}

std::string LLMEngine::run_airllm_inference(const std::string& prompt) {
    if (!airllm_bridge_.is_source_available()) {
        return "AirLLM error: source files not found at " +
               airllm_bridge_.get_config().airllm_root;
    }
    if (airllm_bridge_.get_config().model_id.empty()) {
        return "AirLLM error: no model_id configured";
    }

    // Write the output to a temp file in the build directory
    const std::string output_path = "build/airllm_output.json";

    // Ensure build/ directory exists
    QDir().mkpath(QString::fromStdString("build"));

    const std::string command_str =
        airllm_bridge_.build_inference_command(prompt, output_path);

    // QProcess only accepts program + args separately; split on first space
    // after the quoted exe. For simplicity we run via shell.
    QProcess process;
    process.setProcessChannelMode(QProcess::MergedChannels);

#ifdef Q_OS_WIN
    process.start("cmd.exe", QStringList() << "/c" << QString::fromStdString(command_str));
#else
    process.start("/bin/sh", QStringList() << "-c" << QString::fromStdString(command_str));
#endif

    const int timeout_ms = (airllm_bridge_.get_config().max_new_tokens + 1) * 1500;
    if (!process.waitForFinished(timeout_ms)) {
        process.kill();
        return "AirLLM error: inference timed out after " +
               std::to_string(timeout_ms / 1000) + "s";
    }

    if (process.exitStatus() != QProcess::NormalExit || process.exitCode() != 0) {
        const std::string stderr_text =
            process.readAllStandardOutput().toStdString();
        return "AirLLM error: runner exited with code " +
               std::to_string(process.exitCode()) + ". " + stderr_text;
    }

    // Parse output JSON: {"generated_text": "..."}
    QFile result_file(QString::fromStdString(output_path));
    if (!result_file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return "AirLLM error: could not open output file " + output_path;
    }
    const QByteArray raw = result_file.readAll();
    result_file.close();

    QJsonParseError json_error;
    const QJsonDocument doc = QJsonDocument::fromJson(raw, &json_error);
    if (json_error.error != QJsonParseError::NoError || !doc.isObject()) {
        return "AirLLM error: could not parse output JSON: " +
               json_error.errorString().toStdString();
    }

    const QString generated = doc.object().value("generated_text").toString().trimmed();
    if (generated.isEmpty()) {
        return "AirLLM error: output JSON has no generated_text field";
    }

    return generated.toStdString();
}

} // namespace BrainLLM
