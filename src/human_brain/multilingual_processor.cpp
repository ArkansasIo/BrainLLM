#include "multilingual_processor.h"
#include <algorithm>
#include <cctype>

namespace BrainLLM {

MultiLanguageProcessor::MultiLanguageProcessor() {
    initialize_metadata();
    load_all_vocabularies();
}

Language MultiLanguageProcessor::detect_language(const std::string& text) {
    if (text.empty()) return Language::Unknown;
    
    // Simple heuristics based on common characters
    int latin_count = 0, cyrillic_count = 0, chinese_count = 0, arabic_count = 0;
    
    for (unsigned char c : text) {
        if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z')) {
            latin_count++;
        }
    }
    
    if (latin_count > text.length() * 0.7f) {
        return Language::English;
    }
    
    return Language::Unknown;
}

float MultiLanguageProcessor::get_detection_confidence(const std::string& text, Language lang) {
    Language detected = detect_language(text);
    return (detected == lang) ? 0.8f : 0.2f;
}

ScriptType MultiLanguageProcessor::detect_script(const std::string& text) {
    if (text.empty()) return ScriptType::Unknown;
    
    for (unsigned char c : text) {
        if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z')) {
            return ScriptType::Latin;
        }
    }
    
    return ScriptType::Unknown;
}

LanguageMetadata MultiLanguageProcessor::get_language_metadata(Language lang) {
    auto it = language_metadata_.find(lang);
    if (it != language_metadata_.end()) {
        return it->second;
    }
    return LanguageMetadata();
}

LanguageFeatures MultiLanguageProcessor::get_language_features(Language lang) {
    LanguageFeatures features;
    features.language = lang;
    
    switch (lang) {
        case Language::English:
            features.has_cases = false;
            features.has_gender = false;
            features.has_conjugation = true;
            features.typical_word_order = "SVO";
            break;
        case Language::German:
            features.has_cases = true;
            features.has_gender = true;
            features.has_conjugation = true;
            break;
        case Language::Chinese:
            features.uses_tones = true;
            features.typical_word_order = "SVO";
            break;
        case Language::Japanese:
            features.typical_word_order = "SOV";
            features.uses_particles = true;
            break;
        default:
            break;
    }
    
    return features;
}

std::string MultiLanguageProcessor::translate(const std::string& text,
                                             Language source_lang,
                                             Language target_lang) {
    // Placeholder translation - in production would use ML model
    TranslationContext context;
    context.source_text = text;
    context.source_language = source_lang;
    context.target_language = target_lang;
    context.translated_text = "[Translation of: " + text + "]";
    context.translation_confidence = 0.6f;
    
    last_translation_ = context;
    return context.translated_text;
}

TranslationContext MultiLanguageProcessor::get_translation_context() {
    return last_translation_;
}

std::vector<std::string> MultiLanguageProcessor::get_alternative_translations(int count) {
    std::vector<std::string> alternatives;
    for (int i = 0; i < count; ++i) {
        alternatives.push_back("Alternative " + std::to_string(i + 1));
    }
    return alternatives;
}

std::string MultiLanguageProcessor::normalize_text(const std::string& text, Language lang) {
    std::string normalized = text;
    
    // Convert to lowercase
    std::transform(normalized.begin(), normalized.end(), 
                  normalized.begin(), ::tolower);
    
    return normalized;
}

std::vector<std::string> MultiLanguageProcessor::tokenize_language(const std::string& text, Language lang) {
    std::vector<std::string> tokens;
    std::string current_token;
    
    for (char c : text) {
        if (std::isspace(c)) {
            if (!current_token.empty()) {
                tokens.push_back(current_token);
                current_token.clear();
            }
        } else {
            current_token += c;
        }
    }
    
    if (!current_token.empty()) {
        tokens.push_back(current_token);
    }
    
    return tokens;
}

bool MultiLanguageProcessor::is_valid_character_for_language(char c, Language lang) {
    // Simplified validation
    return std::isalnum(c) || std::isspace(c);
}

std::string MultiLanguageProcessor::get_phonetic_spelling(const std::string& word, Language lang) {
    return "[" + word + "]";
}

std::vector<std::string> MultiLanguageProcessor::get_tones(const std::string& word) {
    std::vector<std::string> tones;
    return tones;
}

MultiLanguageProcessor::GrammarRules MultiLanguageProcessor::get_grammar_rules(Language lang) {
    GrammarRules rules;
    rules.language = lang;
    
    switch (lang) {
        case Language::English:
            rules.rules.push_back("Subject-verb agreement");
            rules.rules.push_back("Proper punctuation");
            rules.rules.push_back("Article usage");
            break;
        default:
            break;
    }
    
    return rules;
}

bool MultiLanguageProcessor::validate_grammar_for_language(const std::string& text, Language lang) {
    return true;
}

void MultiLanguageProcessor::load_vocabulary(Language lang) {
    if (vocabularies_.find(lang) == vocabularies_.end()) {
        vocabularies_[lang] = std::vector<std::string>();
    }
}

std::vector<std::string> MultiLanguageProcessor::get_vocabulary(Language lang, int limit) {
    load_vocabulary(lang);
    auto it = vocabularies_.find(lang);
    if (it != vocabularies_.end()) {
        return std::vector<std::string>(it->second.begin(),
                                       it->second.begin() + 
                                       std::min(limit, (int)it->second.size()));
    }
    return std::vector<std::string>();
}

float MultiLanguageProcessor::get_word_frequency_in_language(const std::string& word, Language lang) {
    return 0.5f;
}

MultiLanguageProcessor::LanguageStats MultiLanguageProcessor::get_language_statistics() {
    return stats_;
}

void MultiLanguageProcessor::initialize_metadata() {
    LanguageMetadata english;
    english.language = Language::English;
    english.language_name = "English";
    english.language_code = "en";
    english.script = ScriptType::Latin;
    english.right_to_left = false;
    language_metadata_[Language::English] = english;
}

void MultiLanguageProcessor::load_all_vocabularies() {
    for (int i = 0; i < static_cast<int>(Language::Unknown); ++i) {
        load_vocabulary(static_cast<Language>(i));
    }
}

} // namespace BrainLLM
