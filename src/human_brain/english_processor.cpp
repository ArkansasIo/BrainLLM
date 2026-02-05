#include "english_processor.h"
#include <algorithm>
#include <cctype>

namespace BrainLLM {

EnglishProcessor::EnglishProcessor() {
    initialize_dictionaries();
    initialize_stop_words();
}

std::vector<std::string> EnglishProcessor::tokenize(const std::string& text) {
    std::vector<std::string> tokens;
    std::string current_token;
    
    for (char c : text) {
        if (std::isspace(c) || c == ',' || c == '.' || c == '!' || c == '?') {
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

std::vector<std::string> EnglishProcessor::split_sentences(const std::string& text) {
    std::vector<std::string> sentences;
    std::string current_sentence;
    
    for (size_t i = 0; i < text.length(); ++i) {
        current_sentence += text[i];
        if (text[i] == '.' || text[i] == '!' || text[i] == '?') {
            if (!current_sentence.empty()) {
                sentences.push_back(current_sentence);
                current_sentence.clear();
            }
        }
    }
    
    if (!current_sentence.empty()) {
        sentences.push_back(current_sentence);
    }
    
    return sentences;
}

WordAnalysis EnglishProcessor::analyze_word(const std::string& word) {
    WordAnalysis analysis;
    analysis.word = word;
    analysis.pos = infer_pos(word);
    analysis.lemma = get_lemma(word);
    analysis.stem = get_stem(word);
    analysis.frequency_score = get_word_frequency(word);
    return analysis;
}

std::vector<WordAnalysis> EnglishProcessor::pos_tag(const std::vector<std::string>& tokens) {
    std::vector<WordAnalysis> tagged;
    for (const auto& token : tokens) {
        tagged.push_back(analyze_word(token));
    }
    return tagged;
}

std::string EnglishProcessor::get_lemma(const std::string& word) {
    auto it = lemma_dictionary_.find(word);
    if (it != lemma_dictionary_.end()) {
        return it->second;
    }
    return word;
}

std::string EnglishProcessor::get_stem(const std::string& word) {
    std::string stem = word;
    
    // Simple stemming rules
    if (stem.length() > 3) {
        if (stem.substr(stem.length() - 3) == "ing") {
            stem = stem.substr(0, stem.length() - 3);
        } else if (stem.substr(stem.length() - 2) == "ed") {
            stem = stem.substr(0, stem.length() - 2);
        } else if (stem.substr(stem.length() - 3) == "ies") {
            stem = stem.substr(0, stem.length() - 3) + "y";
        }
    }
    
    return stem;
}

std::string EnglishProcessor::conjugate_verb(const std::string& verb, Tense tense) {
    std::string conjugated = verb;
    
    switch (tense) {
        case Tense::PresentContinuous:
            conjugated += "ing";
            break;
        case Tense::PastSimple:
            conjugated += "ed";
            break;
        case Tense::FutureSimple:
            conjugated = "will " + conjugated;
            break;
        default:
            break;
    }
    
    return conjugated;
}

std::string EnglishProcessor::pluralize_noun(const std::string& noun) {
    std::string plural = noun;
    
    if (noun.empty()) return plural;
    
    char last_char = noun.back();
    
    if (last_char == 'y') {
        plural = noun.substr(0, noun.length() - 1) + "ies";
    } else if (last_char == 's' || last_char == 'x' || last_char == 'z') {
        plural += "es";
    } else if (last_char == 'o') {
        plural += "es";
    } else {
        plural += "s";
    }
    
    return plural;
}

SentenceStructure EnglishProcessor::parse_sentence(const std::string& sentence) {
    SentenceStructure structure;
    structure.sentence = sentence;
    structure.words = pos_tag(tokenize(sentence));
    structure.tense = detect_tense(structure.words);
    structure.is_question = sentence.back() == '?';
    structure.is_imperative = false;
    structure.is_declarative = !structure.is_question && sentence.back() == '.';
    structure.grammatical_correctness = 0.85f;
    return structure;
}

ParseTree EnglishProcessor::generate_parse_tree(const std::string& sentence) {
    ParseTree tree;
    tree.node_label = "S";
    tree.depth = 0;
    tree.terminal_nodes = pos_tag(tokenize(sentence));
    return tree;
}

bool EnglishProcessor::validate_grammar(const std::string& sentence) {
    auto structure = parse_sentence(sentence);
    return structure.grammatical_correctness > 0.7f;
}

std::string EnglishProcessor::extract_main_idea(const std::string& text) {
    return "Main idea: " + text.substr(0, std::min(size_t(50), text.length()));
}

std::vector<std::string> EnglishProcessor::extract_entities(const std::string& text) {
    std::vector<std::string> entities;
    auto tokens = tokenize(text);
    
    for (const auto& token : tokens) {
        auto analysis = analyze_word(token);
        if (analysis.pos == PartOfSpeech::Noun) {
            entities.push_back(token);
        }
    }
    
    return entities;
}

std::map<std::string, std::string> EnglishProcessor::extract_relations(const std::string& text) {
    std::map<std::string, std::string> relations;
    auto structure = parse_sentence(text);
    
    if (!structure.subject.empty() && !structure.predicate.empty()) {
        relations["subject-predicate"] = structure.subject + " -> " + structure.predicate;
    }
    
    return relations;
}

float EnglishProcessor::analyze_sentiment(const std::string& text) {
    // Simple sentiment: count positive vs negative words
    float sentiment = 0.5f;
    
    if (text.find("good") != std::string::npos) sentiment += 0.1f;
    if (text.find("bad") != std::string::npos) sentiment -= 0.1f;
    if (text.find("happy") != std::string::npos) sentiment += 0.15f;
    if (text.find("sad") != std::string::npos) sentiment -= 0.15f;
    
    return std::min(1.0f, std::max(0.0f, sentiment));
}

float EnglishProcessor::calculate_readability(const std::string& text) {
    auto sentences = split_sentences(text);
    auto tokens = tokenize(text);
    
    if (sentences.empty() || tokens.empty()) return 0.5f;
    
    float avg_sentence_length = tokens.size() / (float)sentences.size();
    float avg_word_length = 0.0f;
    
    for (const auto& token : tokens) {
        avg_word_length += token.length();
    }
    avg_word_length /= tokens.size();
    
    // Flesch Reading Ease approximation
    float readability = 206.835f - 1.015f * avg_sentence_length - 84.6f * (avg_word_length / 4.7f);
    return std::min(1.0f, std::max(0.0f, readability / 100.0f));
}

std::string EnglishProcessor::classify_text_style(const std::string& text) {
    float readability = calculate_readability(text);
    
    if (readability > 0.7f) return "formal";
    if (readability > 0.4f) return "neutral";
    return "informal";
}

std::string EnglishProcessor::generate_synonym_phrase(const std::string& phrase) {
    return "similar to: " + phrase;
}

std::string EnglishProcessor::expand_sentence(const std::string& sentence) {
    return sentence + " with more details...";
}

std::string EnglishProcessor::simplify_text(const std::string& text) {
    return text;
}

float EnglishProcessor::get_word_frequency(const std::string& word) {
    auto it = word_frequencies_.find(word);
    if (it != word_frequencies_.end()) {
        return it->second;
    }
    return 0.1f;
}

std::vector<std::string> EnglishProcessor::get_similar_words(const std::string& word, int count) {
    std::vector<std::string> similar;
    return similar;
}

void EnglishProcessor::initialize_dictionaries() {
    pos_dictionary_["the"] = PartOfSpeech::Determiner;
    pos_dictionary_["is"] = PartOfSpeech::Verb;
    pos_dictionary_["cat"] = PartOfSpeech::Noun;
    lemma_dictionary_["running"] = "run";
    lemma_dictionary_["walked"] = "walk";
    word_frequencies_["the"] = 0.95f;
}

void EnglishProcessor::initialize_stop_words() {
    common_stop_words_ = {"the", "a", "an", "and", "or", "but", "in", "on", "at", "to"};
}

PartOfSpeech EnglishProcessor::infer_pos(const std::string& word) {
    auto it = pos_dictionary_.find(word);
    if (it != pos_dictionary_.end()) {
        return it->second;
    }
    
    if (word.back() == 'y' || word.back() == 'e') {
        return PartOfSpeech::Adjective;
    }
    
    return PartOfSpeech::Noun;
}

Tense EnglishProcessor::detect_tense(const std::vector<WordAnalysis>& words) {
    for (const auto& word : words) {
        if (word.word == "is" || word.word == "am" || word.word == "are") {
            return Tense::PresentSimple;
        }
        if (word.word == "was" || word.word == "were") {
            return Tense::PastSimple;
        }
    }
    return Tense::Unknown;
}

} // namespace BrainLLM
