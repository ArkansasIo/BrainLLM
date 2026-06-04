#pragma once

#include "brain_types.h"
#include <string>
#include <vector>
#include <map>

namespace BrainLLM {

// ========================================
// ENGLISH LANGUAGE PROCESSING
// ========================================

enum class PartOfSpeech {
    Noun,
    Verb,
    Adjective,
    Adverb,
    Pronoun,
    Preposition,
    Conjunction,
    Determiner,
    Interjection,
    Unknown
};

enum class Tense {
    PresentSimple,
    PresentContinuous,
    PresentPerfect,
    PastSimple,
    PastContinuous,
    PastPerfect,
    FutureSimple,
    FutureFormative,
    Conditional,
    Unknown
};

struct WordAnalysis {
    std::string word;
    PartOfSpeech pos;
    std::string lemma;
    std::string stem;
    std::vector<std::string> morphological_features;
    float frequency_score;
    std::vector<std::string> synonyms;
    std::vector<std::string> antonyms;
};

struct SentenceStructure {
    std::string sentence;
    std::vector<WordAnalysis> words;
    std::string subject;
    std::string predicate;
    std::string object;
    Tense tense;
    bool is_question;
    bool is_imperative;
    bool is_declarative;
    float grammatical_correctness;
    std::string semantic_meaning;
};

struct ParseTree {
    std::string node_label;
    std::vector<WordAnalysis> terminal_nodes;
    std::vector<ParseTree> subtrees;
    int depth;
};

class EnglishProcessor {
public:
    EnglishProcessor();
    ~EnglishProcessor() = default;
    
    // Tokenization
    std::vector<std::string> tokenize(const std::string& text);
    std::vector<std::string> split_sentences(const std::string& text);
    
    // Part-of-speech tagging
    WordAnalysis analyze_word(const std::string& word);
    std::vector<WordAnalysis> pos_tag(const std::vector<std::string>& tokens);
    
    // Morphological analysis
    std::string get_lemma(const std::string& word);
    std::string get_stem(const std::string& word);
    std::string conjugate_verb(const std::string& verb, Tense tense);
    std::string pluralize_noun(const std::string& noun);
    
    // Syntax analysis
    SentenceStructure parse_sentence(const std::string& sentence);
    ParseTree generate_parse_tree(const std::string& sentence);
    bool validate_grammar(const std::string& sentence);
    
    // Semantic analysis
    std::string extract_main_idea(const std::string& text);
    std::vector<std::string> extract_entities(const std::string& text);
    std::map<std::string, std::string> extract_relations(const std::string& text);
    
    // Sentiment and style
    float analyze_sentiment(const std::string& text);
    float calculate_readability(const std::string& text);
    std::string classify_text_style(const std::string& text);
    
    // Text generation
    std::string generate_synonym_phrase(const std::string& phrase);
    std::string expand_sentence(const std::string& sentence);
    std::string simplify_text(const std::string& text);
    
    // Vocabulary
    float get_word_frequency(const std::string& word);
    std::vector<std::string> get_similar_words(const std::string& word, int count = 5);
    
private:
    std::map<std::string, PartOfSpeech> pos_dictionary_;
    std::map<std::string, std::string> lemma_dictionary_;
    std::map<std::string, float> word_frequencies_;
    std::vector<std::string> common_stop_words_;
    
    void initialize_dictionaries();
    void initialize_stop_words();
    
    PartOfSpeech infer_pos(const std::string& word);
    Tense detect_tense(const std::vector<WordAnalysis>& words);
};

} // namespace BrainLLM
