#include "multilingual_processor.h"
#include <algorithm>
#include <cctype>
#include <map>
#include <sstream>

namespace BrainLLM {

MultiLanguageProcessor::MultiLanguageProcessor() {
    initialize_metadata();
    load_all_vocabularies();
}

Language MultiLanguageProcessor::detect_language(const std::string& text) {
    if (text.empty()) return Language::Unknown;

    std::string lower = normalize_text(text, Language::English);
    if (lower.find(" el ") != std::string::npos || lower.find(" la ") != std::string::npos ||
        lower.find(" que ") != std::string::npos || lower.find("hola") != std::string::npos) {
        return Language::Spanish;
    }
    if (lower.find(" le ") != std::string::npos || lower.find(" la ") != std::string::npos ||
        lower.find("bonjour") != std::string::npos || lower.find("merci") != std::string::npos) {
        return Language::French;
    }
    if (lower.find(" der ") != std::string::npos || lower.find(" die ") != std::string::npos ||
        lower.find(" und ") != std::string::npos || lower.find("hallo") != std::string::npos) {
        return Language::German;
    }

    int latin_count = 0;
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
    static const std::map<std::string, std::map<Language, std::string>> phrasebook = {
        {"hello", {{Language::Spanish, "hola"}, {Language::French, "bonjour"}, {Language::German, "hallo"}}},
        {"thank you", {{Language::Spanish, "gracias"}, {Language::French, "merci"}, {Language::German, "danke"}}},
        {"goodbye", {{Language::Spanish, "adios"}, {Language::French, "au revoir"}, {Language::German, "auf wiedersehen"}}},
        {"brain", {{Language::Spanish, "cerebro"}, {Language::French, "cerveau"}, {Language::German, "gehirn"}}},
        {"learning", {{Language::Spanish, "aprendizaje"}, {Language::French, "apprentissage"}, {Language::German, "lernen"}}}
    };

    std::string translated = text;
    std::string lower = normalize_text(text, source_lang);
    auto phrase = phrasebook.find(lower);
    if (phrase != phrasebook.end()) {
        auto target = phrase->second.find(target_lang);
        if (target != phrase->second.end()) {
            translated = target->second;
        }
    } else if (source_lang == target_lang) {
        translated = text;
    } else {
        translated = text + " [" + get_language_metadata(target_lang).language_code + "]";
    }

    TranslationContext context;
    context.source_text = text;
    context.source_language = source_lang;
    context.target_language = target_lang;
    context.translated_text = translated;
    context.translation_confidence = (translated == text && source_lang != target_lang) ? 0.35f : 0.75f;
    context.alternative_translations = {translated, text};

    last_translation_ = context;
    return translated;
}

TranslationContext MultiLanguageProcessor::get_translation_context() {
    return last_translation_;
}

std::vector<std::string> MultiLanguageProcessor::get_alternative_translations(int count) {
    std::vector<std::string> alternatives = last_translation_.alternative_translations;
    while (alternatives.size() < static_cast<size_t>(count)) {
        alternatives.push_back(last_translation_.translated_text);
    }
    if (alternatives.size() > static_cast<size_t>(count)) {
        alternatives.resize(count);
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
    if (text.empty()) return false;
    if (lang == Language::English || lang == Language::Spanish || lang == Language::French || lang == Language::German) {
        char last = text.back();
        return last == '.' || last == '!' || last == '?';
    }
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
    load_vocabulary(lang);
    const auto& vocabulary = vocabularies_[lang];
    return std::find(vocabulary.begin(), vocabulary.end(), normalize_text(word, lang)) != vocabulary.end() ? 0.75f : 0.2f;
}

MultiLanguageProcessor::LanguageStats MultiLanguageProcessor::get_language_statistics() {
    return stats_;
}

void MultiLanguageProcessor::initialize_metadata() {
    auto add = [this](Language lang, const std::string& name, const std::string& code,
                      ScriptType script, bool rtl, float avg_len) {
        LanguageMetadata metadata;
        metadata.language = lang;
        metadata.language_name = name;
        metadata.language_code = code;
        metadata.script = script;
        metadata.right_to_left = rtl;
        metadata.avg_word_length = avg_len;
        metadata.unique_characters = 26;
        language_metadata_[lang] = metadata;
    };

    add(Language::English, "English", "en", ScriptType::Latin, false, 5.1f);
    add(Language::Spanish, "Spanish", "es", ScriptType::Latin, false, 5.0f);
    add(Language::French, "French", "fr", ScriptType::Latin, false, 5.2f);
    add(Language::German, "German", "de", ScriptType::Latin, false, 6.1f);
    add(Language::Chinese, "Chinese", "zh", ScriptType::Chinese, false, 1.5f);
    add(Language::Japanese, "Japanese", "ja", ScriptType::Japanese, false, 2.0f);
    add(Language::Arabic, "Arabic", "ar", ScriptType::Arabic, true, 4.8f);
}

void MultiLanguageProcessor::load_all_vocabularies() {
    for (int i = 0; i < static_cast<int>(Language::Unknown); ++i) {
        load_vocabulary(static_cast<Language>(i));
    }
    vocabularies_[Language::English] = {"hello", "brain", "learning", "memory", "attention", "model"};
    vocabularies_[Language::Spanish] = {"hola", "cerebro", "aprendizaje", "memoria", "modelo"};
    vocabularies_[Language::French] = {"bonjour", "cerveau", "apprentissage", "memoire", "modele"};
    vocabularies_[Language::German] = {"hallo", "gehirn", "lernen", "speicher", "modell"};
}

} // namespace BrainLLM
