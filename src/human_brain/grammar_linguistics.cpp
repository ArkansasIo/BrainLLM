#include "grammar_linguistics.h"
#include <algorithm>
#include <cctype>
#include <sstream>

namespace BrainLLM {

GrammarAnalyzer::GrammarAnalyzer() {
    initialize_grammar_rules();
}

std::vector<GrammarError> GrammarAnalyzer::check_grammar(const std::string& text) {
    std::vector<GrammarError> errors;

    if (text.empty()) {
        errors.push_back({0, "empty_text", "", "Provide text to analyze.", 1.0f, "No grammar can be analyzed without text."});
        return errors;
    }

    if (!check_punctuation(text)) {
        errors.push_back({static_cast<int>(text.size() - 1), "missing_terminal_punctuation", text.substr(text.size() - 1), "End with '.', '!', or '?'.", 0.85f, "Complete sentences need terminal punctuation."});
    }

    auto subject_verb = detect_subject_verb_disagreement(text);
    if (subject_verb.confidence > 0.0f) errors.push_back(subject_verb);

    auto comma_splice = detect_comma_splice(text);
    if (comma_splice.confidence > 0.0f) errors.push_back(comma_splice);

    return errors;
}

bool GrammarAnalyzer::is_grammatically_correct(const std::string& text) {
    auto errors = check_grammar(text);
    return errors.empty();
}

float GrammarAnalyzer::calculate_grammar_score(const std::string& text) {
    auto errors = check_grammar(text);
    float score = 1.0f - (errors.size() * 0.1f);
    return std::max(0.0f, std::min(1.0f, score));
}

GrammarError GrammarAnalyzer::detect_subject_verb_disagreement(const std::string& sentence) {
    GrammarError error;
    error.position = 0;
    error.error_type = "subject_verb_disagreement";
    error.confidence = 0.0f;

    std::string lower = sentence;
    std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);
    if (lower.find(" i is ") != std::string::npos || lower.find(" they is ") != std::string::npos ||
        lower.find(" we is ") != std::string::npos || lower.find(" you is ") != std::string::npos) {
        error.problematic_text = sentence;
        error.suggestion = "Use 'am' with I and 'are' with you/we/they.";
        error.explanation = "The subject and verb do not agree in number/person.";
        error.confidence = 0.9f;
    }
    return error;
}

GrammarError GrammarAnalyzer::detect_pronoun_errors(const std::string& sentence) {
    GrammarError error;
    error.error_type = "pronoun_error";
    error.confidence = 0.0f;
    return error;
}

GrammarError GrammarAnalyzer::detect_tense_inconsistency(const std::string& sentence) {
    GrammarError error;
    error.error_type = "tense_inconsistency";
    error.confidence = 0.0f;
    return error;
}

GrammarError GrammarAnalyzer::detect_dangling_modifier(const std::string& sentence) {
    GrammarError error;
    error.error_type = "dangling_modifier";
    error.confidence = 0.0f;
    return error;
}

GrammarError GrammarAnalyzer::detect_comma_splice(const std::string& sentence) {
    GrammarError error;
    error.error_type = "comma_splice";
    error.confidence = 0.0f;
    if (sentence.find(", and") == std::string::npos && sentence.find(", but") == std::string::npos &&
        sentence.find(", so") == std::string::npos && sentence.find(", ") != std::string::npos) {
        error.position = static_cast<int>(sentence.find(", "));
        error.problematic_text = ",";
        error.suggestion = "Use a period, semicolon, or coordinating conjunction.";
        error.explanation = "A comma may be joining two independent clauses.";
        error.confidence = 0.55f;
    }
    return error;
}

std::vector<SyntacticRelation> GrammarAnalyzer::analyze_dependencies(const std::string& sentence) {
    std::vector<SyntacticRelation> relations;
    std::istringstream iss(sentence);
    std::vector<std::string> words;
    std::string word;
    while (iss >> word) words.push_back(word);
    if (words.size() >= 2) {
        relations.push_back({words[1], words[0], "subject", 0.65f});
    }
    if (words.size() >= 3) {
        relations.push_back({words[1], words[2], "object_or_complement", 0.55f});
    }
    return relations;
}

GrammarTag GrammarAnalyzer::identify_phrase_type(const std::string& phrase) {
    return GrammarTag::Other;
}

ParserResult GrammarAnalyzer::parse_with_grammar_rules(const std::string& sentence) {
    ParserResult result;
    result.success = is_grammatically_correct(sentence);
    result.root_tag = GrammarTag::S;
    result.relations = analyze_dependencies(sentence);
    for (const auto& error : check_grammar(sentence)) {
        result.errors.push_back(error.error_type + ": " + error.suggestion);
    }
    return result;
}

std::vector<SemanticRole> GrammarAnalyzer::label_semantic_roles(const std::string& sentence) {
    std::vector<SemanticRole> roles;
    return roles;
}

bool GrammarAnalyzer::check_punctuation(const std::string& text) {
    if (text.empty()) return false;
    char c = text.back();
    return c == '.' || c == '!' || c == '?';
}

std::vector<std::string> GrammarAnalyzer::get_punctuation_suggestions(const std::string& text) {
    std::vector<std::string> suggestions;
    if (!check_punctuation(text)) {
        suggestions.push_back("Add terminal punctuation.");
    }
    if (text.find(",,") != std::string::npos) {
        suggestions.push_back("Remove duplicated commas.");
    }
    return suggestions;
}

GrammarAnalyzer::AdvancedGrammarFeatures GrammarAnalyzer::analyze_advanced_features(const std::string& text) {
    AdvancedGrammarFeatures features;
    features.has_parallel_structure = false;
    features.uses_active_voice = true;
    features.uses_concrete_nouns = true;
    features.lexical_diversity = 0.7f;
    features.sentence_variety_score = 65;
    return features;
}

void GrammarAnalyzer::add_custom_rule(const GrammarRule& rule) {
    custom_rules_[rule.rule_name] = rule;
}

std::vector<GrammarRule> GrammarAnalyzer::get_applicable_rules(const std::string& text) {
    std::vector<GrammarRule> applicable;
    for (const auto& rule : grammar_rules_) {
        if (text.find(rule.pattern) != std::string::npos || rule.pattern.find(" ") != std::string::npos) {
            applicable.push_back(rule);
        }
    }
    for (const auto& [name, rule] : custom_rules_) {
        if (text.find(rule.pattern) != std::string::npos) {
            applicable.push_back(rule);
        }
    }
    return applicable;
}

void GrammarAnalyzer::initialize_grammar_rules() {
    GrammarRule rule;
    rule.rule_name = "subject_verb_agreement";
    rule.pattern = "Subject must agree with verb in number";
    rule.violation_frequency = 0.15f;
    grammar_rules_.push_back(rule);
}

bool GrammarAnalyzer::check_subject_verb_agreement(const std::string& subject, const std::string& verb) {
    return true;
}

bool GrammarAnalyzer::check_pronoun_antecedent_agreement(const std::string& pronoun, const std::string& antecedent) {
    return true;
}

// ========================================
// LINGUISTIC ANALYZER
// ========================================

LinguisticAnalyzer::LinguisticAnalyzer() {
    initialize_features();
}

std::vector<LinguisticFeature> LinguisticAnalyzer::extract_linguistic_features(const std::string& text) {
    std::vector<LinguisticFeature> features;
    
    LinguisticFeature feature;
    feature.feature_name = "word_count";
    feature.value = text.length() / 5.0f;  // Approximate word count
    feature.category = "lexical";
    features.push_back(feature);
    
    return features;
}

StyleAnalysis LinguisticAnalyzer::analyze_style(const std::string& text) {
    StyleAnalysis analysis;
    analysis.formality_score = 0.6f;
    analysis.readability_grade = 8.0f;
    analysis.complexity_score = 0.5f;
    analysis.clarity_score = 0.75f;
    analysis.engagement_score = 0.65f;
    analysis.detected_styles.push_back("neutral");
    return analysis;
}

LinguisticAnalyzer::DiscourseStructure LinguisticAnalyzer::analyze_discourse(const std::string& text) {
    DiscourseStructure structure;
    structure.text_type = "expository";
    structure.overall_structure = "linear";
    structure.coherence_score = 0.8f;
    return structure;
}

LinguisticAnalyzer::Stylometry LinguisticAnalyzer::calculate_stylometry(const std::string& text) {
    Stylometry stylometry;
    stylometry.avg_word_length = 5.0f;
    stylometry.avg_sentence_length = 15.0f;
    stylometry.type_token_ratio = 0.6f;
    stylometry.hapax_legomena_ratio = 0.1f;
    return stylometry;
}

LinguisticAnalyzer::RhetoricAnalysis LinguisticAnalyzer::analyze_rhetoric(const std::string& text) {
    RhetoricAnalysis analysis;
    analysis.persuasiveness_score = 0.5f;
    return analysis;
}

LinguisticAnalyzer::PhonologicalAnalysis LinguisticAnalyzer::analyze_phonology(const std::string& text) {
    PhonologicalAnalysis analysis;
    analysis.phonetic_complexity = 0.4f;
    return analysis;
}

void LinguisticAnalyzer::initialize_features() {
    LinguisticFeature feature;
    feature.feature_name = "lexical_density";
    feature.category = "lexical";
    linguistic_features_.push_back(feature);
}

} // namespace BrainLLM
