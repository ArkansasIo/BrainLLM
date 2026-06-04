#pragma once

#include "brain_types.h"
#include <string>
#include <vector>
#include <map>

namespace BrainLLM {

// ========================================
// MULTI-LANGUAGE SUPPORT
// ========================================

enum class Language {
    English,
    Spanish,
    French,
    German,
    Italian,
    Portuguese,
    Russian,
    Chinese,           // Mandarin
    Japanese,
    Korean,
    Arabic,
    Hindi,
    Turkish,
    Dutch,
    Swedish,
    Polish,
    Vietnamese,
    Thai,
    Hebrew,
    Farsi,
    Unknown
};

enum class ScriptType {
    Latin,
    Cyrillic,
    Greek,
    Arabic,
    Chinese,
    Japanese,
    Korean,
    Devanagari,
    Thai,
    Hebrew,
    Unknown
};

struct LanguageMetadata {
    Language language;
    std::string language_name;
    std::string language_code;     // ISO 639-1
    ScriptType script;
    bool right_to_left;
    std::vector<std::string> phonemes;
    std::map<std::string, int> character_frequencies;
    float avg_word_length;
    int unique_characters;
};

struct TranslationContext {
    std::string source_text;
    Language source_language;
    Language target_language;
    std::string translated_text;
    float translation_confidence;
    std::vector<std::string> alternative_translations;
    std::map<std::string, std::string> word_mappings;
};

struct LanguageFeatures {
    Language language;
    int num_vowels;
    int num_consonants;
    bool has_cases;
    bool has_gender;
    bool has_conjugation;
    int typical_sentence_length;
    std::string typical_word_order;  // SVO, SOV, VSO, etc.
    bool uses_particles;
    bool uses_tones;
};

class MultiLanguageProcessor {
public:
    MultiLanguageProcessor();
    ~MultiLanguageProcessor() = default;
    
    // Language detection
    Language detect_language(const std::string& text);
    float get_detection_confidence(const std::string& text, Language lang);
    ScriptType detect_script(const std::string& text);
    
    // Language metadata
    LanguageMetadata get_language_metadata(Language lang);
    LanguageFeatures get_language_features(Language lang);
    
    // Translation
    std::string translate(const std::string& text, 
                         Language source_lang, 
                         Language target_lang);
    TranslationContext get_translation_context();
    std::vector<std::string> get_alternative_translations(int count = 3);
    
    // Character and script handling
    std::string normalize_text(const std::string& text, Language lang);
    std::vector<std::string> tokenize_language(const std::string& text, Language lang);
    bool is_valid_character_for_language(char c, Language lang);
    
    // Language-specific analysis
    std::string get_phonetic_spelling(const std::string& word, Language lang);
    std::vector<std::string> get_tones(const std::string& word);  // For tonal languages
    
    // Grammar rules by language
    struct GrammarRules {
        Language language;
        std::vector<std::string> rules;
        std::map<std::string, std::string> exceptions;
    };
    
    GrammarRules get_grammar_rules(Language lang);
    bool validate_grammar_for_language(const std::string& text, Language lang);
    
    // Vocabulary management
    void load_vocabulary(Language lang);
    std::vector<std::string> get_vocabulary(Language lang, int limit = 1000);
    float get_word_frequency_in_language(const std::string& word, Language lang);
    
    // Language statistics
    struct LanguageStats {
        Language language;
        int total_words_processed;
        std::map<Language, int> cross_language_usage;
        float dominant_language_percentage;
    };
    
    LanguageStats get_language_statistics();
    
private:
    std::map<Language, LanguageMetadata> language_metadata_;
    std::map<Language, std::vector<std::string>> vocabularies_;
    TranslationContext last_translation_;
    LanguageStats stats_;
    
    void initialize_metadata();
    void load_all_vocabularies();
};

} // namespace BrainLLM
