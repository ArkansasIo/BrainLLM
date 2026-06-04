#include "integrated_cognitive_module.h"
#include <algorithm>
#include <cctype>
#include <sstream>

namespace BrainLLM {

namespace {

std::string trim_copy(std::string value) {
    auto not_space = [](unsigned char c) { return !std::isspace(c); };
    value.erase(value.begin(), std::find_if(value.begin(), value.end(), not_space));
    value.erase(std::find_if(value.rbegin(), value.rend(), not_space).base(), value.end());
    return value;
}

std::vector<std::string> keywords_from_text(const std::string& text, size_t limit = 6) {
    static const std::vector<std::string> stop_words = {
        "the", "and", "for", "that", "this", "with", "from", "you", "your",
        "are", "was", "were", "have", "has", "had", "what", "why", "how"
    };

    std::vector<std::string> keywords;
    std::istringstream stream(text);
    std::string word;
    while (stream >> word) {
        word.erase(std::remove_if(word.begin(), word.end(), [](unsigned char c) {
            return !std::isalnum(c);
        }), word.end());
        std::transform(word.begin(), word.end(), word.begin(), ::tolower);
        if (word.size() < 3 || std::find(stop_words.begin(), stop_words.end(), word) != stop_words.end()) {
            continue;
        }
        if (std::find(keywords.begin(), keywords.end(), word) == keywords.end()) {
            keywords.push_back(word);
        }
        if (keywords.size() >= limit) {
            break;
        }
    }
    return keywords;
}

} // namespace

IntegratedCognitiveModule::IntegratedCognitiveModule()
    : verbose_logging_(false) {
    cognitive_processor_ = std::make_unique<CognitiveProcessor>();
    english_processor_ = std::make_unique<EnglishProcessor>();
    multilingual_processor_ = std::make_unique<MultiLanguageProcessor>();
    grammar_analyzer_ = std::make_unique<GrammarAnalyzer>();
    linguistic_analyzer_ = std::make_unique<LinguisticAnalyzer>();
}

IntegratedCognitiveModule::~IntegratedCognitiveModule() = default;

void IntegratedCognitiveModule::add_thought(const std::string& thought) {
    cognitive_processor_->add_to_working_memory(thought);
    thought_frequency_[thought]++;
    recent_topics_.push_back(thought);
    
    if (verbose_logging_) {
        log_cognitive_activity("Added thought: " + thought);
    }
}

void IntegratedCognitiveModule::focus_attention(const std::string& topic, float intensity) {
    cognitive_processor_->set_attention_focus(topic, intensity);
    
    if (verbose_logging_) {
        log_cognitive_activity("Focus attention on: " + topic + " (intensity: " + std::to_string(intensity) + ")");
    }
}

std::string IntegratedCognitiveModule::think_and_decide(const std::vector<std::string>& options,
                                                       const std::map<std::string, float>& option_values) {
    DecisionContext context;
    context.options = options;
    context.option_values = option_values;
    context.time_pressure = 0.2f;
    context.risk_tolerance = 0.3f;
    
    std::string decision = cognitive_processor_->make_decision(context);
    
    if (verbose_logging_) {
        log_cognitive_activity("Made decision: " + decision);
    }
    
    return decision;
}

std::vector<std::string> IntegratedCognitiveModule::recall_related_thoughts(const std::string& cue) {
    auto memories = cognitive_processor_->retrieve_related_memories(cue);
    std::vector<std::string> thoughts;
    
    for (const auto& memory : memories) {
        thoughts.push_back(memory.content);
    }
    
    return thoughts;
}

IntegratedCognitiveModule::CognitiveState IntegratedCognitiveModule::get_cognitive_state() const {
    CognitiveState state;
    state.working_memory_load = cognitive_processor_->get_working_memory_load();
    state.attention_focus = cognitive_processor_->get_attention_state().focus_target;
    state.attention_intensity = cognitive_processor_->get_attention_state().focus_intensity;
    state.is_overloaded = cognitive_processor_->is_cognitive_overload();
    return state;
}

IntegratedCognitiveModule::EnglishAnalysis IntegratedCognitiveModule::analyze_english_text(const std::string& text) {
    EnglishAnalysis analysis;
    
    // Sentence splitting
    analysis.sentences = english_processor_->split_sentences(text);
    
    // POS tagging
    auto tokens = english_processor_->tokenize(text);
    analysis.pos_tags = english_processor_->pos_tag(tokens);
    
    // Sentiment and readability
    analysis.sentiment = english_processor_->analyze_sentiment(text);
    analysis.readability = english_processor_->calculate_readability(text);
    analysis.style = english_processor_->classify_text_style(text);
    
    // Entity extraction
    analysis.entities = english_processor_->extract_entities(text);
    
    // Main idea
    analysis.main_idea = english_processor_->extract_main_idea(text);
    
    return analysis;
}

bool IntegratedCognitiveModule::check_english_grammar(const std::string& sentence) {
    return english_processor_->validate_grammar(sentence);
}

std::string IntegratedCognitiveModule::generate_english_sentence(const std::vector<std::string>& words) {
    std::string sentence;
    for (size_t i = 0; i < words.size(); ++i) {
        sentence += words[i];
        if (i < words.size() - 1) sentence += " ";
    }
    if (!sentence.empty()) sentence += ".";
    return sentence;
}

IntegratedCognitiveModule::LanguageAnalysis IntegratedCognitiveModule::analyze_multilingual_text(const std::string& text) {
    LanguageAnalysis analysis;
    
    analysis.language = multilingual_processor_->detect_language(text);
    analysis.detection_confidence = multilingual_processor_->get_detection_confidence(text, analysis.language);
    analysis.features = multilingual_processor_->get_language_features(analysis.language);
    analysis.normalized_tokens = multilingual_processor_->tokenize_language(
        multilingual_processor_->normalize_text(text, analysis.language),
        analysis.language
    );
    
    return analysis;
}

IntegratedCognitiveModule::TranslationResult IntegratedCognitiveModule::translate_text(
    const std::string& text,
    const std::string& target_language) {
    
    TranslationResult result;
    
    // Detect source language
    auto source_lang = multilingual_processor_->detect_language(text);
    
    // Map target language string to enum
    Language target_lang = Language::English;
    if (target_language == "es") target_lang = Language::Spanish;
    else if (target_language == "fr") target_lang = Language::French;
    else if (target_language == "de") target_lang = Language::German;
    else if (target_language == "zh") target_lang = Language::Chinese;
    
    result.translated_text = multilingual_processor_->translate(text, source_lang, target_lang);
    result.source_language = source_lang;
    result.target_language = target_lang;
    result.translation_confidence = 0.7f;
    result.alternatives = multilingual_processor_->get_alternative_translations(3);
    
    return result;
}

LanguageMetadata IntegratedCognitiveModule::get_language_info(const std::string& language_code) {
    Language lang = Language::English;
    if (language_code == "es") lang = Language::Spanish;
    else if (language_code == "fr") lang = Language::French;
    
    return multilingual_processor_->get_language_metadata(lang);
}

IntegratedCognitiveModule::LinguisticAnalysis IntegratedCognitiveModule::full_linguistic_analysis(const std::string& text) {
    LinguisticAnalysis analysis;
    
    // Grammar check
    analysis.grammar_errors = grammar_analyzer_->check_grammar(text);
    analysis.grammar_score = grammar_analyzer_->calculate_grammar_score(text);
    
    // Linguistic analysis
    analysis.style = linguistic_analyzer_->analyze_style(text);
    analysis.stylometry = linguistic_analyzer_->calculate_stylometry(text);
    analysis.rhetoric = linguistic_analyzer_->analyze_rhetoric(text);
    analysis.features = linguistic_analyzer_->extract_linguistic_features(text);
    
    return analysis;
}

IntegratedCognitiveModule::GrammarCorrection IntegratedCognitiveModule::check_and_correct_grammar(const std::string& text) {
    GrammarCorrection correction;
    
    correction.errors = grammar_analyzer_->check_grammar(text);
    correction.has_errors = !correction.errors.empty();
    correction.corrected_text = trim_copy(text);

    for (const auto& error : correction.errors) {
        if (error.error_type == "missing_terminal_punctuation" && !correction.corrected_text.empty()) {
            correction.corrected_text += ".";
        } else if (error.error_type == "subject_verb_disagreement") {
            std::string lower = correction.corrected_text;
            std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);
            const std::vector<std::pair<std::string, std::string>> replacements = {
                {" i is ", " I am "},
                {" they is ", " they are "},
                {" we is ", " we are "},
                {" you is ", " you are "}
            };
            for (const auto& [needle, replacement] : replacements) {
                const size_t pos = lower.find(needle);
                if (pos != std::string::npos) {
                    correction.corrected_text.replace(pos, needle.size(), replacement);
                    break;
                }
            }
        }
    }
    
    return correction;
}

IntegratedCognitiveModule::ProcessedInput IntegratedCognitiveModule::understand_user_input(const std::string& user_text) {
    ProcessedInput input;
    
    // Detect language
    input.detected_language = multilingual_processor_->detect_language(user_text);
    
    // Always provide English analysis (translate if needed)
    if (input.detected_language != Language::English) {
        auto translation = translate_text(user_text, "en");
        input.english_analysis = analyze_english_text(translation.translated_text);
    } else {
        input.english_analysis = analyze_english_text(user_text);
    }
    
    // Extract understanding
    input.understood_meaning = input.english_analysis.main_idea;
    input.understanding_confidence = calculate_understanding_confidence(user_text);
    input.key_topics = input.english_analysis.entities;
    if (input.key_topics.empty()) {
        input.key_topics = keywords_from_text(user_text);
    }
    
    if (verbose_logging_) {
        log_cognitive_activity("Understood input: " + input.understood_meaning);
    }
    
    return input;
}

IntegratedCognitiveModule::GeneratedResponse IntegratedCognitiveModule::generate_intelligent_response(
    const ProcessedInput& input,
    const std::string& response_language) {
    
    GeneratedResponse response;
    
    // Generate base response
    response.response_text = "Responding to: " + input.understood_meaning;
    response.response_language = Language::English;
    response.quality_score = 0.75f;
    response.grammatically_correct = true;
    response.reasoning = "Processed understood meaning and generated appropriate response";
    
    // Translate if needed
    if (response_language != "en") {
        auto translation = translate_text(response.response_text, response_language);
        response.response_text = translation.translated_text;
    }
    
    if (verbose_logging_) {
        log_cognitive_activity("Generated response: " + response.response_text);
    }
    
    return response;
}

void IntegratedCognitiveModule::sleep_and_consolidate() {
    cognitive_processor_->simulate_sleep_cycle();
    if (verbose_logging_) {
        log_cognitive_activity("Sleep cycle completed - memories consolidated");
    }
}

void IntegratedCognitiveModule::increase_cognitive_load(float amount) {
    const int synthetic_items = std::max(1, static_cast<int>(amount * 4.0f));
    for (int i = 0; i < synthetic_items; ++i) {
        cognitive_processor_->add_to_working_memory("cognitive-load:" + std::to_string(amount) + ":" + std::to_string(i));
    }
    if (verbose_logging_) {
        log_cognitive_activity("Cognitive load increased by " + std::to_string(amount));
    }
}

void IntegratedCognitiveModule::reset_cognitive_state() {
    cognitive_processor_->clear_working_memory();
    thought_frequency_.clear();
    recent_topics_.clear();
    
    if (verbose_logging_) {
        log_cognitive_activity("Cognitive state reset");
    }
}

void IntegratedCognitiveModule::enable_verbose_logging(bool enable) {
    verbose_logging_ = enable;
}

void IntegratedCognitiveModule::log_cognitive_activity(const std::string& activity) {
    // In production, would write to logging system
    // For now, just storing in memory
}

float IntegratedCognitiveModule::calculate_understanding_confidence(const std::string& text) {
    float confidence = 0.5f;
    
    // Simple heuristic: longer, more detailed text suggests higher confidence
    if (text.length() > 100) confidence += 0.2f;
    if (text.find("?") != std::string::npos) confidence -= 0.1f;  // Questions are less confident
    
    return std::min(1.0f, std::max(0.0f, confidence));
}

} // namespace BrainLLM
