#include "llm_engine.h"
#include <algorithm>
#include <cctype>
#include <map>
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

bool contains_any(const std::set<std::string>& words, const std::vector<std::string>& candidates) {
    for (const auto& candidate : candidates) {
        if (words.find(candidate) != words.end()) {
            return true;
        }
    }
    return false;
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

} // namespace

LLMEngine::LLMEngine(const BrainConfig& config)
    : config_(config), state_(BrainState::Idle), confidence_(0.0f) {
    neural_net_ = std::make_unique<NeuralNetwork>(config);
    memory_ = std::make_unique<MemorySystem>(config.max_memory_size);
    attention_ = std::make_unique<AttentionMechanism>(config.num_attention_heads, config.embedding_dim);
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

    auto words = words_from(prompt);
    auto memories = memory_->retrieve_memories(prompt, 3);

    std::ostringstream response;
    response << "BrainLLM response: ";

    if (prompt.empty()) {
        response << "provide a prompt and I will generate a response.";
        confidence_ = 0.25f;
    } else {
        response << "I understand the prompt as focused on " << join_keywords(words) << ". ";
        if (!memories.empty()) {
            response << "Relevant memory: " << first_sentence(memories[0].content) << " ";
        }
        response << "Recommended answer: " << first_sentence(prompt);
        response << " The useful path is to clarify the goal, identify constraints, execute the smallest working step, and verify the result.";
        confidence_ = memories.empty() ? 0.70f : 0.80f;
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

} // namespace BrainLLM
