#pragma once

#include "brain_types.h"
#include <string>
#include <vector>
#include <map>

namespace BrainLLM {

// ========================================
// ADVANCED GRAMMAR AND LINGUISTICS
// ========================================

enum class GrammarType {
    Phrase,
    Clause,
    Sentence,
    Paragraph,
    Discourse,
    Unknown
};

enum class GrammarTag {
    // Noun phrases
    NP,
    // Verb phrases
    VP,
    // Prepositional phrases
    PP,
    // Adjective phrases
    AP,
    // Adverb phrases
    AdvP,
    // Clauses
    S,
    NounClause,
    AdjectiveClause,
    AdverbClause,
    // Other
    Other
};

struct GrammarRule {
    std::string rule_name;
    std::string pattern;
    std::vector<std::string> examples;
    float violation_frequency;
    std::string correction;
};

struct GrammarError {
    int position;
    std::string error_type;
    std::string problematic_text;
    std::string suggestion;
    float confidence;
    std::string explanation;
};

struct SyntacticRelation {
    std::string head;
    std::string dependent;
    std::string relation_type;  // Subject, Object, Modifier, etc.
    float strength;
};

struct SemanticRole {
    std::string word;
    std::string role;  // Agent, Patient, Theme, Instrument, Locative, etc.
    float confidence;
};

class GrammarAnalyzer {
public:
    GrammarAnalyzer();
    ~GrammarAnalyzer() = default;
    
    // Grammar checking
    std::vector<GrammarError> check_grammar(const std::string& text);
    bool is_grammatically_correct(const std::string& text);
    float calculate_grammar_score(const std::string& text);
    
    // Error detection and correction
    GrammarError detect_subject_verb_disagreement(const std::string& sentence);
    GrammarError detect_pronoun_errors(const std::string& sentence);
    GrammarError detect_tense_inconsistency(const std::string& sentence);
    GrammarError detect_dangling_modifier(const std::string& sentence);
    GrammarError detect_comma_splice(const std::string& sentence);
    
    // Syntax analysis
    std::vector<SyntacticRelation> analyze_dependencies(const std::string& sentence);
    GrammarTag identify_phrase_type(const std::string& phrase);
    ParserResult parse_with_grammar_rules(const std::string& sentence);
    
    // Semantic role labeling
    std::vector<SemanticRole> label_semantic_roles(const std::string& sentence);
    
    // Punctuation analysis
    bool check_punctuation(const std::string& text);
    std::vector<std::string> get_punctuation_suggestions(const std::string& text);
    
    // Advanced features
    struct AdvancedGrammarFeatures {
        bool has_parallel_structure;
        bool uses_active_voice;
        bool uses_concrete_nouns;
        float lexical_diversity;
        int sentence_variety_score;
        std::vector<std::string> rhetorical_devices;
    };
    
    AdvancedGrammarFeatures analyze_advanced_features(const std::string& text);
    
    // Grammar rules management
    void add_custom_rule(const GrammarRule& rule);
    std::vector<GrammarRule> get_applicable_rules(const std::string& text);
    
private:
    std::vector<GrammarRule> grammar_rules_;
    std::map<std::string, GrammarRule> custom_rules_;
    
    void initialize_grammar_rules();
    bool check_subject_verb_agreement(const std::string& subject, const std::string& verb);
    bool check_pronoun_antecedent_agreement(const std::string& pronoun, const std::string& antecedent);
};

// ========================================
// LINGUISTIC ANALYSIS
// ========================================

struct LinguisticFeature {
    std::string feature_name;
    float value;
    std::string category;
};

struct StyleAnalysis {
    float formality_score;           // 0 = casual, 1 = formal
    float readability_grade;         // Grade level needed to understand
    float complexity_score;          // Sentence and vocabulary complexity
    float clarity_score;             // How clear and understandable
    float engagement_score;          // How interesting to reader
    std::vector<std::string> detected_styles;  // Active, passive, etc.
};

class LinguisticAnalyzer {
public:
    LinguisticAnalyzer();
    ~LinguisticAnalyzer() = default;
    
    // Basic linguistic features
    std::vector<LinguisticFeature> extract_linguistic_features(const std::string& text);
    
    // Style analysis
    StyleAnalysis analyze_style(const std::string& text);
    
    // Discourse analysis
    struct DiscourseStructure {
        std::string text_type;  // Narrative, Expository, Persuasive, etc.
        std::vector<std::string> discourse_markers;
        std::string overall_structure;
        float coherence_score;
    };
    
    DiscourseStructure analyze_discourse(const std::string& text);
    
    // Stylometric analysis
    struct Stylometry {
        float avg_word_length;
        float avg_sentence_length;
        int unique_word_count;
        float type_token_ratio;  // Vocabulary richness
        std::map<std::string, float> function_word_frequencies;
        float hapax_legomena_ratio;  // Words appearing once
    };
    
    Stylometry calculate_stylometry(const std::string& text);
    
    // Rhetoric and persuasion
    struct RhetoricAnalysis {
        std::vector<std::string> rhetorical_devices;
        std::vector<std::string> logical_appeals;
        std::vector<std::string> emotional_appeals;
        float persuasiveness_score;
    };
    
    RhetoricAnalysis analyze_rhetoric(const std::string& text);
    
    // Phonological analysis
    struct PhonologicalAnalysis {
        std::vector<std::string> alliteration;
        std::vector<std::string> assonance;
        std::vector<std::string> rhymes;
        float phonetic_complexity;
    };
    
    PhonologicalAnalysis analyze_phonology(const std::string& text);
    
private:
    std::vector<LinguisticFeature> linguistic_features_;
    std::map<std::string, float> function_words_;
    
    void initialize_features();
};

} // namespace BrainLLM
