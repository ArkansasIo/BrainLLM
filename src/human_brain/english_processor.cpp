#include "english_processor.h"
#include <algorithm>
#include <cctype>
#include <set>
#include <sstream>

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
    structure.is_question = !sentence.empty() && sentence.back() == '?';
    structure.is_imperative = !structure.words.empty() && structure.words.front().pos == PartOfSpeech::Verb;
    structure.is_declarative = !structure.is_question && !sentence.empty() && sentence.back() == '.';

    for (const auto& word : structure.words) {
        if (structure.subject.empty() && (word.pos == PartOfSpeech::Noun || word.pos == PartOfSpeech::Pronoun)) {
            structure.subject = word.word;
        } else if (structure.predicate.empty() && word.pos == PartOfSpeech::Verb) {
            structure.predicate = word.word;
        } else if (!structure.predicate.empty() && structure.object.empty() && word.pos == PartOfSpeech::Noun) {
            structure.object = word.word;
        }
    }

    structure.grammatical_correctness = 0.65f;
    if (!structure.subject.empty()) structure.grammatical_correctness += 0.1f;
    if (!structure.predicate.empty()) structure.grammatical_correctness += 0.15f;
    if (structure.is_question || structure.is_declarative || structure.is_imperative) structure.grammatical_correctness += 0.1f;
    structure.grammatical_correctness = std::min(1.0f, structure.grammatical_correctness);
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
    auto sentences = split_sentences(text);
    if (!sentences.empty()) {
        return sentences.front();
    }
    return text.substr(0, std::min(size_t(80), text.length()));
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
    std::map<std::string, std::string> synonyms = {
        {"good", "strong"}, {"bad", "weak"}, {"fast", "quick"},
        {"smart", "intelligent"}, {"big", "large"}, {"small", "compact"}
    };
    auto tokens = tokenize(phrase);
    for (auto& token : tokens) {
        std::string lower = token;
        std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);
        auto it = synonyms.find(lower);
        if (it != synonyms.end()) {
            token = it->second;
        }
    }

    std::ostringstream oss;
    for (size_t i = 0; i < tokens.size(); ++i) {
        if (i > 0) oss << " ";
        oss << tokens[i];
    }
    return oss.str();
}

std::string EnglishProcessor::expand_sentence(const std::string& sentence) {
    return sentence + " This means the idea should be evaluated with context, constraints, and expected outcomes.";
}

std::string EnglishProcessor::simplify_text(const std::string& text) {
    auto sentences = split_sentences(text);
    if (sentences.empty()) return text;
    return sentences.front();
}

float EnglishProcessor::get_word_frequency(const std::string& word) {
    auto it = word_frequencies_.find(word);
    if (it != word_frequencies_.end()) {
        return it->second;
    }
    return 0.1f;
}

std::vector<std::string> EnglishProcessor::get_similar_words(const std::string& word, int count) {
    std::map<std::string, std::vector<std::string>> dictionary = {
        {"ai", {"assistant", "model", "agent", "automation", "reasoner"}},
        {"brain", {"cognition", "memory", "attention", "reasoning", "learning"}},
        {"fast", {"quick", "rapid", "speedy", "efficient", "responsive"}},
        {"good", {"strong", "useful", "effective", "reliable", "sound"}}
    };
    std::string lower = word;
    std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);
    auto it = dictionary.find(lower);
    std::vector<std::string> similar = (it != dictionary.end()) ? it->second : std::vector<std::string>{word};
    if (similar.size() > static_cast<size_t>(count)) {
        similar.resize(count);
    }
    return similar;
}

void EnglishProcessor::initialize_dictionaries() {
    // ========== PART-OF-SPEECH DICTIONARY ==========
    // Determiners
    for (const auto& w : {"the", "a", "an", "this", "that", "these", "those",
                          "my", "your", "his", "her", "its", "our", "their",
                          "each", "every", "either", "neither", "some", "any",
                          "no", "all", "both", "few", "little", "many", "much",
                          "more", "most", "other", "another", "enough", "several",
                          "what", "which", "whatever", "whichever"}) {
        pos_dictionary_[w] = PartOfSpeech::Determiner;
    }
    // Pronouns
    for (const auto& w : {"i", "me", "my", "mine", "myself",
                          "you", "your", "yours", "yourself", "yourselves",
                          "he", "him", "himself", "she", "her", "herself",
                          "it", "itself", "we", "us", "ourselves",
                          "they", "them", "their", "theirs", "themselves",
                          "who", "whom", "whose", "which", "that",
                          "someone", "anyone", "everyone", "no one",
                          "something", "anything", "everything", "nothing",
                          "one", "ones"}) {
        pos_dictionary_[w] = PartOfSpeech::Pronoun;
    }
    // Common verbs
    for (const auto& w : {"be", "is", "am", "are", "was", "were", "been", "being",
                          "have", "has", "had", "having", "do", "does", "did", "done",
                          "go", "goes", "went", "gone", "get", "gets", "got",
                          "make", "makes", "made", "know", "knows", "knew", "known",
                          "think", "thinks", "thought", "see", "sees", "saw", "seen",
                          "come", "comes", "came", "want", "wants", "wanted",
                          "look", "looks", "looked", "use", "uses", "used",
                          "find", "finds", "found", "give", "gives", "gave", "given",
                          "tell", "tells", "told", "work", "works", "worked",
                          "call", "calls", "called", "try", "tries", "tried",
                          "ask", "asks", "asked", "need", "needs", "needed",
                          "feel", "feels", "felt", "become", "becomes", "became",
                          "leave", "leaves", "left", "put", "puts", "show",
                          "shows", "showed", "shown", "hear", "hears", "heard",
                          "play", "plays", "played", "run", "runs", "ran",
                          "move", "moves", "moved", "live", "lives", "lived",
                          "believe", "believes", "believed", "hold", "holds", "held",
                          "bring", "brings", "brought", "happen", "happens", "happened",
                          "write", "writes", "wrote", "provide", "provides", "provided",
                          "sit", "sits", "sat", "stand", "stands", "stood",
                          "lose", "loses", "lost", "pay", "pays", "paid",
                          "meet", "meets", "met", "include", "includes", "included",
                          "continue", "continues", "continued", "set", "sets",
                          "learn", "learns", "learned", "change", "changes", "changed",
                          "lead", "leads", "led", "understand", "understands", "understood",
                          "watch", "watches", "watched", "follow", "follows", "followed",
                          "stop", "stops", "stopped", "create", "creates", "created",
                          "speak", "speaks", "spoke", "spoken", "read", "reads",
                          "spend", "spends", "spent", "grow", "grows", "grew",
                          "open", "opens", "opened", "walk", "walks", "walked",
                          "win", "wins", "won", "offer", "offers", "offered",
                          "remember", "remembers", "remembered", "love", "loves", "loved",
                          "consider", "considers", "considered", "appear", "appears",
                          "appeared", "buy", "buys", "bought", "wait", "waits", "waited",
                          "serve", "serves", "served", "die", "dies", "died",
                          "send", "sends", "sent", "expect", "expects", "expected",
                          "build", "builds", "built", "stay", "stays", "stayed",
                          "fall", "falls", "fell", "cut", "cuts", "reach",
                          "reaches", "reached", "kill", "kills", "killed",
                          "remain", "remains", "remained", "suggest", "suggests",
                          "suggested", "raise", "raises", "raised", "pass",
                          "passes", "passed", "sell", "sells", "sold",
                          "require", "requires", "required", "report", "reports",
                          "reported", "decide", "decides", "decided",
                          "pull", "pulls", "pulled", "process", "processes",
                          "generate", "generates", "generated", "train", "trains",
                          "trained", "compute", "computes", "computed",
                          "analyze", "analyzes", "analyzed", "detect", "detects",
                          "detected", "initialize", "initializes", "initialized",
                          "reset", "resets", "configure", "configures", "configured",
                          "execute", "executes", "executed", "deploy", "deploys",
                          "deployed", "store", "stores", "stored", "retrieve",
                          "retrieves", "retrieved", "update", "updates", "updated",
                          "query", "queries", "queried", "extract", "extracts",
                          "extracted", "parse", "parses", "parsed",
                          "classify", "classifies", "classified", "predict",
                          "predicts", "predicted", "optimize", "optimizes",
                          "optimized", "evaluate", "evaluates", "evaluated",
                          "test", "tests", "tested", "compile", "compiles",
                          "compiled", "load", "loads", "loaded", "save",
                          "saves", "saved", "export", "exports", "exported",
                          "import", "imports", "imported"}) {
        pos_dictionary_[w] = PartOfSpeech::Verb;
    }
    // Common nouns
    for (const auto& w : {"time", "year", "people", "way", "day", "man", "woman",
                          "child", "world", "life", "hand", "part", "place", "case",
                          "week", "company", "system", "program", "question", "number",
                          "night", "point", "home", "water", "room", "mother",
                          "area", "money", "story", "fact", "month", "lot",
                          "right", "study", "book", "eye", "job", "word",
                          "business", "issue", "side", "kind", "head", "house",
                          "service", "friend", "father", "power", "hour", "game",
                          "line", "end", "among", "order", "name", "son",
                          "thought", "body", "family", "face", "level", "field",
                          "girl", "air", "door", "table", "group", "problem",
                          "city", "state", "car", "town", "example", "idea",
                          "body", "country", "school", "moment", "office",
                          "action", "voice", "light", "reason", "matter",
                          "science", "technology", "computer", "software", "hardware",
                          "network", "internet", "data", "database", "server",
                          "client", "application", "interface", "algorithm",
                          "model", "training", "inference", "output", "input",
                          "token", "embedding", "layer", "weight", "gradient",
                          "loss", "accuracy", "precision", "recall", "bias",
                          "attention", "transformer", "encoder", "decoder",
                          "neuron", "activation", "function", "parameter",
                          "dataset", "batch", "epoch", "learning", "knowledge",
                          "intelligence", "brain", "mind", "memory", "language",
                          "text", "sentence", "word", "phrase", "grammar",
                          "syntax", "semantics", "context", "meaning", "concept",
                          "category", "class", "type", "object", "entity",
                          "attribute", "relation", "graph", "node", "edge",
                          "api", "endpoint", "request", "response", "message",
                          "error", "exception", "warning", "log", "debug",
                          "version", "release", "build", "test", "feature",
                          "module", "library", "framework", "platform", "cloud",
                          "task", "process", "thread", "queue", "stack",
                          "key", "value", "map", "list", "array", "string",
                          "integer", "float", "boolean", "character", "byte",
                          "file", "directory", "path", "url", "protocol",
                          "security", "authentication", "encryption", "token",
                          "user", "admin", "permission", "role", "session",
                          "wolfram", "quantum", "physics", "chemistry", "math",
                          "equation", "formula", "variable", "constant", "unit",
                          "energy", "force", "velocity", "mass", "temperature",
                          "voltage", "current", "frequency", "wavelength", "particle",
                          "atom", "molecule", "cell", "gene", "protein",
                          "evolution", "ecosystem", "climate", "environment",
                          "history", "culture", "society", "economy", "politics",
                          "art", "music", "literature", "philosophy", "ethics"}) {
        pos_dictionary_[w] = PartOfSpeech::Noun;
    }
    // Common adjectives
    for (const auto& w : {"good", "new", "first", "last", "long", "great", "little",
                          "own", "old", "right", "big", "high", "different", "small",
                          "large", "next", "early", "young", "important", "public",
                          "private", "real", "best", "free", "sure", "better",
                          "true", "hard", "strong", "specific", "clear", "clear",
                          "available", "able", "simple", "complex", "full", "open",
                          "local", "global", "general", "social", "economic",
                          "political", "historical", "cultural", "scientific",
                          "technical", "digital", "physical", "natural", "human",
                          "main", "major", "significant", "possible", "likely",
                          "final", "current", "recent", "particular", "common",
                          "various", "certain", "single", "central", "complete",
                          "direct", "basic", "effective", "efficient", "modern",
                          "positive", "negative", "active", "passive", "dynamic",
                          "static", "stable", "volatile", "fast", "slow",
                          "accurate", "precise", "approximate", "exact", "optimal",
                          "minimal", "maximal", "critical", "essential", "optional",
                          "required", "valid", "invalid", "correct", "incorrect",
                          "successful", "failed", "enabled", "disabled",
                          "intelligent", "smart", "creative", "logical", "rational",
                          "emotional", "cognitive", "neural", "quantum", "classical",
                          "linear", "nonlinear", "recursive", "iterative"}) {
        pos_dictionary_[w] = PartOfSpeech::Adjective;
    }
    // Common adverbs
    for (const auto& w : {"not", "also", "very", "often", "however", "too", "usually",
                          "really", "early", "never", "always", "sometimes", "together",
                          "likely", "simply", "generally", "instead", "actually",
                          "still", "yet", "well", "already", "soon", "just",
                          "then", "now", "here", "there", "where", "when",
                          "how", "why", "so", "more", "most", "less", "least",
                          "quite", "rather", "almost", "nearly", "enough",
                          "quickly", "slowly", "correctly", "incorrectly",
                          "efficiently", "effectively", "accurately", "precisely",
                          "directly", "indirectly", "automatically", "manually",
                          "recursively", "iteratively", "dynamically", "statically"}) {
        pos_dictionary_[w] = PartOfSpeech::Adverb;
    }
    // Prepositions
    for (const auto& w : {"in", "on", "at", "by", "for", "with", "about", "against",
                          "between", "into", "through", "during", "before", "after",
                          "above", "below", "from", "up", "down", "out", "off",
                          "over", "under", "again", "further", "then", "once",
                          "to", "of", "near", "among", "within", "without",
                          "across", "along", "behind", "beside", "beyond",
                          "inside", "outside", "around", "towards", "upon",
                          "except", "per", "until", "since", "unlike"}) {
        pos_dictionary_[w] = PartOfSpeech::Preposition;
    }
    // Conjunctions
    for (const auto& w : {"and", "or", "but", "if", "while", "although", "because",
                          "since", "unless", "until", "when", "where", "whether",
                          "after", "before", "that", "though", "even", "as",
                          "nor", "yet", "so", "either", "neither", "both",
                          "not only", "but also", "provided", "assuming"}) {
        pos_dictionary_[w] = PartOfSpeech::Conjunction;
    }

    // ========== LEMMA DICTIONARY ==========
    // Irregular verbs
    const std::vector<std::pair<std::string,std::string>> lemmas = {
        {"am","be"},{"is","be"},{"are","be"},{"was","be"},{"were","be"},
        {"been","be"},{"being","be"},
        {"has","have"},{"had","have"},{"having","have"},
        {"does","do"},{"did","do"},{"done","do"},
        {"goes","go"},{"went","go"},{"gone","go"},
        {"gets","get"},{"got","get"},{"gotten","get"},
        {"makes","make"},{"made","make"},
        {"knows","know"},{"knew","know"},{"known","know"},
        {"thinks","think"},{"thought","think"},
        {"sees","see"},{"saw","see"},{"seen","see"},
        {"comes","come"},{"came","come"},
        {"takes","take"},{"took","take"},{"taken","take"},
        {"gives","give"},{"gave","give"},{"given","give"},
        {"tells","tell"},{"told","tell"},
        {"finds","find"},{"found","find"},
        {"says","say"},{"said","say"},
        {"brings","bring"},{"brought","bring"},
        {"writes","write"},{"wrote","write"},{"written","write"},
        {"runs","run"},{"ran","run"},
        {"speaks","speak"},{"spoke","speak"},{"spoken","speak"},
        {"reads","read"},
        {"leads","lead"},{"led","lead"},
        {"builds","build"},{"built","build"},
        {"loses","lose"},{"lost","lose"},
        {"wins","win"},{"won","win"},
        {"pays","pay"},{"paid","pay"},
        {"sells","sell"},{"sold","sell"},
        {"sends","send"},{"sent","send"},
        {"holds","hold"},{"held","hold"},
        {"meets","meet"},{"met","meet"},
        {"sits","sit"},{"sat","sit"},
        {"stands","stand"},{"stood","stand"},
        {"falls","fall"},{"fell","fall"},
        {"cuts","cut"},
        {"puts","put"},
        {"sets","set"},
        {"lets","let"},
        {"hits","hit"},
        {"fits","fit"},
        {"growing","grow"},{"grew","grow"},{"grown","grow"},
        {"running","run"},
        {"walking","walk"},{"walked","walk"},
        {"talking","talk"},{"talked","talk"},
        {"working","work"},{"worked","work"},
        {"called","call"},{"calling","call"},
        {"using","use"},{"used","use"},
        {"needed","need"},{"needing","need"},
        {"training","train"},{"trained","train"},
        {"learning","learn"},{"learned","learn"},
        {"processing","process"},{"processed","process"},
        {"generating","generate"},{"generated","generate"},
        {"computing","compute"},{"computed","compute"},
        {"analyzing","analyze"},{"analyzed","analyze"},
        {"detecting","detect"},{"detected","detect"},
        {"storing","store"},{"stored","store"},
        {"retrieving","retrieve"},{"retrieved","retrieve"},
        {"updating","update"},{"updated","update"},
        {"querying","query"},{"queried","query"},
        {"parsing","parse"},{"parsed","parse"},
        {"classifying","classify"},{"classified","classify"},
        {"predicting","predict"},{"predicted","predict"},
        {"optimizing","optimize"},{"optimized","optimize"},
        {"evaluating","evaluate"},{"evaluated","evaluate"},
        {"testing","test"},{"tested","test"},
        {"loading","load"},{"loaded","load"},
        {"saving","save"},{"saved","save"},
        {"children","child"},{"men","man"},{"women","woman"},
        {"people","person"},{"feet","foot"},{"teeth","tooth"},
        {"mice","mouse"},{"geese","goose"},{"oxen","ox"},
        {"data","datum"},{"criteria","criterion"},{"phenomena","phenomenon"},
        {"analyses","analysis"},{"bases","basis"},{"indices","index"},
        {"matrices","matrix"},{"vertices","vertex"},{"formulae","formula"},
        {"better","good"},{"best","good"},{"worse","bad"},{"worst","bad"},
        {"more","much"},{"most","much"},{"less","little"},{"least","little"},
        {"further","far"},{"furthest","far"},{"farther","far"},{"farthest","far"}
    };
    for (const auto& [form, base] : lemmas) {
        lemma_dictionary_[form] = base;
    }

    // ========== WORD FREQUENCIES (0.0 - 1.0) ==========
    const std::vector<std::pair<std::string,float>> freqs = {
        {"the",0.98f},{"of",0.95f},{"and",0.94f},{"a",0.93f},{"to",0.92f},
        {"in",0.91f},{"is",0.90f},{"it",0.89f},{"you",0.88f},{"that",0.87f},
        {"he",0.86f},{"was",0.85f},{"for",0.84f},{"on",0.83f},{"are",0.82f},
        {"with",0.81f},{"as",0.80f},{"at",0.79f},{"be",0.78f},{"by",0.77f},
        {"this",0.76f},{"have",0.75f},{"from",0.74f},{"or",0.73f},{"one",0.72f},
        {"had",0.71f},{"not",0.70f},{"but",0.69f},{"what",0.68f},{"all",0.67f},
        {"were",0.66f},{"we",0.65f},{"when",0.64f},{"your",0.63f},{"can",0.62f},
        {"said",0.61f},{"there",0.60f},{"use",0.59f},{"an",0.58f},{"each",0.57f},
        {"which",0.56f},{"she",0.55f},{"do",0.54f},{"how",0.53f},{"their",0.52f},
        {"if",0.51f},{"will",0.50f},{"up",0.49f},{"other",0.48f},{"about",0.47f},
        {"out",0.46f},{"many",0.45f},{"then",0.44f},{"so",0.43f},{"some",0.42f},
        {"time",0.65f},{"word",0.60f},{"number",0.58f},{"no",0.50f},{"way",0.55f},
        {"could",0.52f},{"people",0.60f},{"my",0.55f},{"than",0.50f},{"first",0.48f},
        {"water",0.45f},{"been",0.60f},{"call",0.40f},{"who",0.55f},{"its",0.52f},
        {"now",0.50f},{"find",0.45f},{"long",0.45f},{"down",0.43f},{"day",0.50f},
        {"did",0.50f},{"get",0.52f},{"come",0.50f},{"made",0.48f},{"may",0.52f},
        {"part",0.45f},{"over",0.47f},{"new",0.55f},{"sound",0.40f},{"take",0.50f},
        {"only",0.55f},{"little",0.45f},{"work",0.55f},{"know",0.55f},{"place",0.48f},
        {"year",0.55f},{"live",0.48f},{"me",0.52f},{"back",0.48f},{"give",0.48f},
        {"most",0.50f},{"very",0.55f},{"after",0.52f},{"thing",0.48f},{"our",0.52f},
        {"just",0.55f},{"name",0.48f},{"good",0.58f},{"sentence",0.40f},{"man",0.52f},
        // Technology and AI specific
        {"ai",0.70f},{"machine",0.60f},{"learning",0.68f},{"neural",0.65f},
        {"network",0.65f},{"deep",0.60f},{"model",0.68f},{"data",0.70f},
        {"algorithm",0.62f},{"computer",0.65f},{"software",0.62f},{"code",0.65f},
        {"function",0.60f},{"class",0.58f},{"object",0.58f},{"method",0.58f},
        {"memory",0.62f},{"attention",0.60f},{"transformer",0.62f},
        {"language",0.65f},{"natural",0.60f},{"processing",0.60f},
        {"training",0.62f},{"inference",0.60f},{"token",0.58f},
        {"embedding",0.58f},{"vector",0.55f},{"matrix",0.55f},
        {"weight",0.55f},{"layer",0.55f},{"gradient",0.52f},{"loss",0.52f},
        {"accuracy",0.55f},{"precision",0.52f},{"recall",0.50f},
        {"brain",0.68f},{"cognitive",0.60f},{"intelligence",0.65f},
        {"wolfram",0.55f},{"quantum",0.55f},{"physics",0.55f}
    };
    for (const auto& [w, f] : freqs) {
        word_frequencies_[w] = f;
    }

    // ========== SYNONYMS & ANTONYMS per WordAnalysis usage ==========
    // These are stored inline and returned from get_similar_words / generate_synonym_phrase.
    // (Actual synonym lookup reuses the extended_synonyms_ map built below.)

    // Build extended synonym map
    extended_synonyms_["good"] = {"great","excellent","fine","solid","sound","useful","effective","reliable","strong"};
    extended_synonyms_["bad"] = {"poor","weak","flawed","faulty","defective","harmful","negative","inferior"};
    extended_synonyms_["fast"] = {"quick","rapid","speedy","swift","efficient","responsive","instant","prompt"};
    extended_synonyms_["slow"] = {"sluggish","delayed","gradual","leisurely","unhurried","lagging"};
    extended_synonyms_["big"] = {"large","great","huge","enormous","vast","substantial","sizable","major"};
    extended_synonyms_["small"] = {"tiny","little","minor","compact","slight","minimal","microscopic"};
    extended_synonyms_["smart"] = {"intelligent","clever","bright","sharp","capable","astute","knowledgeable"};
    extended_synonyms_["simple"] = {"easy","basic","straightforward","clear","plain","uncomplicated","elementary"};
    extended_synonyms_["complex"] = {"complicated","intricate","elaborate","advanced","sophisticated","multifaceted"};
    extended_synonyms_["important"] = {"significant","critical","essential","key","vital","major","fundamental"};
    extended_synonyms_["new"] = {"novel","recent","modern","fresh","current","latest","innovative","updated"};
    extended_synonyms_["old"] = {"ancient","dated","outdated","legacy","obsolete","prior","previous","classic"};
    extended_synonyms_["true"] = {"correct","accurate","valid","factual","real","genuine","verified"};
    extended_synonyms_["false"] = {"incorrect","inaccurate","invalid","wrong","erroneous","mistaken","untrue"};
    extended_synonyms_["start"] = {"begin","initiate","launch","commence","trigger","activate","initialize"};
    extended_synonyms_["stop"] = {"end","halt","cease","terminate","finish","conclude","abort","disable"};
    extended_synonyms_["create"] = {"build","make","construct","generate","produce","develop","design","form"};
    extended_synonyms_["delete"] = {"remove","erase","drop","destroy","eliminate","clear","purge","wipe"};
    extended_synonyms_["find"] = {"locate","discover","detect","identify","search","retrieve","uncover"};
    extended_synonyms_["show"] = {"display","present","reveal","demonstrate","exhibit","render","output"};
    extended_synonyms_["use"] = {"apply","employ","utilize","leverage","operate","invoke","run","execute"};
    extended_synonyms_["change"] = {"modify","update","alter","adjust","transform","revise","edit","replace"};
    extended_synonyms_["increase"] = {"grow","raise","expand","amplify","boost","enhance","multiply","scale"};
    extended_synonyms_["decrease"] = {"reduce","lower","shrink","diminish","drop","cut","minimize","compress"};
    extended_synonyms_["understand"] = {"comprehend","grasp","know","recognize","interpret","perceive","realize"};
    extended_synonyms_["explain"] = {"describe","clarify","elaborate","detail","illustrate","define","interpret"};
    extended_synonyms_["analyze"] = {"examine","study","evaluate","assess","inspect","investigate","review"};
    extended_synonyms_["calculate"] = {"compute","solve","determine","derive","evaluate","estimate","measure"};
    extended_synonyms_["connect"] = {"link","join","integrate","associate","combine","bind","attach","wire"};
    extended_synonyms_["ai"] = {"assistant","model","agent","automation","reasoner","bot","system","engine"};
    extended_synonyms_["brain"] = {"cognition","memory","attention","reasoning","learning","mind","intellect"};
    extended_synonyms_["model"] = {"network","system","framework","architecture","engine","module","agent"};
    extended_synonyms_["data"] = {"information","content","input","record","dataset","corpus","payload"};
    extended_synonyms_["error"] = {"bug","fault","issue","defect","failure","exception","problem","mistake"};
    extended_synonyms_["response"] = {"reply","answer","output","result","feedback","message","return"};
    extended_synonyms_["query"] = {"question","request","prompt","input","search","lookup","inquiry"};
    extended_synonyms_["memory"] = {"storage","cache","recall","record","retention","history","context"};
    extended_synonyms_["language"] = {"tongue","dialect","speech","text","code","notation","syntax"};
    extended_synonyms_["knowledge"] = {"information","facts","wisdom","expertise","understanding","learning"};
    extended_synonyms_["process"] = {"method","procedure","routine","function","operation","workflow","pipeline"};
    extended_synonyms_["system"] = {"framework","platform","infrastructure","environment","network","engine"};
    extended_synonyms_["function"] = {"method","routine","procedure","operation","capability","feature","tool"};
    extended_synonyms_["test"] = {"evaluate","verify","check","validate","assess","examine","confirm"};
    extended_synonyms_["security"] = {"safety","protection","defense","privacy","encryption","authentication"};
    extended_synonyms_["efficient"] = {"optimal","effective","fast","lean","streamlined","productive","capable"};
    extended_synonyms_["accurate"] = {"precise","correct","exact","valid","faithful","reliable","true"};
    extended_synonyms_["input"] = {"data","prompt","request","query","feed","signal","source"};
    extended_synonyms_["output"] = {"result","response","answer","product","return","generation","text"};
}

void EnglishProcessor::initialize_stop_words() {
    common_stop_words_ = {
        "the","a","an","and","or","but","in","on","at","to","of","for",
        "is","are","was","were","be","been","it","this","that","i","you",
        "we","he","she","they","do","did","have","has","had","will","would",
        "can","could","should","may","might","shall","must","with","from",
        "by","as","if","so","not","no","nor","yet","both","either","neither",
        "each","every","all","more","most","less","few","many","much","some",
        "any","other","such","own","same","just","then","than","too","very"
    };
}

PartOfSpeech EnglishProcessor::infer_pos(const std::string& word) {
    auto it = pos_dictionary_.find(word);
    if (it != pos_dictionary_.end()) {
        return it->second;
    }
    
    if (word.empty()) return PartOfSpeech::Unknown;
    if (word.size() > 3 && (word.substr(word.size() - 3) == "ing" || word.substr(word.size() - 2) == "ed")) {
        return PartOfSpeech::Verb;
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
