#pragma once

#include "brain_types.h"
#include <string>
#include <vector>
#include <map>

namespace BrainLLM {

// ========================================
// KNOWLEDGE GRAPH AND REASONING
// ========================================

struct KnowledgeNode {
    std::string id;
    std::string label;
    std::string category;
    std::map<std::string, float> attributes;
    std::vector<std::string> connected_nodes;
};

struct Relationship {
    std::string source_id;
    std::string target_id;
    std::string type;
    float strength;
    std::string context;
};

class KnowledgeGraph {
public:
    KnowledgeGraph();
    ~KnowledgeGraph() = default;
    
    // Node operations
    void add_node(const KnowledgeNode& node);
    void remove_node(const std::string& node_id);
    KnowledgeNode get_node(const std::string& node_id) const;
    
    // Relationship operations
    void add_relationship(const Relationship& rel);
    std::vector<Relationship> find_relationships(const std::string& node_id) const;
    
    // Reasoning
    std::vector<std::string> infer_knowledge(const std::string& query);
    std::vector<std::string> find_reasoning_path(const std::string& start, 
                                                const std::string& end);
    
    // Query and search
    std::vector<KnowledgeNode> search_nodes(const std::string& query);
    float measure_knowledge_connectivity() const;
    
private:
    std::map<std::string, KnowledgeNode> nodes_;
    std::vector<Relationship> relationships_;
};

// ========================================
// REASONING AND LOGIC
// ========================================

enum class LogicType {
    Deductive,      // General → Specific
    Inductive,      // Specific → General
    Abductive,      // Observation → Explanation
    Analogical,     // Similar → Similar
    Causal          // Cause → Effect
};

struct LogicalStatement {
    std::string statement;
    bool is_true;
    float confidence;
    LogicType logic_type;
    std::vector<std::string> supporting_evidence;
};

class ReasoningEngine {
public:
    ReasoningEngine();
    ~ReasoningEngine() = default;
    
    // Reasoning operations
    bool evaluate_statement(const std::string& statement);
    std::vector<std::string> deduce_conclusions(const std::vector<std::string>& premises);
    std::string generate_explanation(const std::string& phenomenon);
    
    // Logical inference
    bool is_logically_consistent(const std::vector<std::string>& statements) const;
    std::vector<LogicalStatement> analyze_logic(const std::string& text);
    
    // Uncertainty handling
    float calculate_confidence(const LogicalStatement& statement);
    
private:
    std::vector<LogicalStatement> reasoning_history_;
    float confidence_threshold_;
};

// ========================================
// SEMANTIC ANALYSIS
// ========================================

struct SemanticVector {
    std::vector<float> components;
    std::string word;
    float magnitude;
};

class SemanticAnalyzer {
public:
    SemanticAnalyzer();
    ~SemanticAnalyzer() = default;
    
    // Semantic operations
    SemanticVector vectorize(const std::string& word);
    float semantic_similarity(const std::string& word1, const std::string& word2);
    std::vector<std::string> find_synonyms(const std::string& word);
    std::vector<std::string> find_antonyms(const std::string& word);
    
    // Meaning extraction
    std::string extract_meaning(const std::string& text);
    std::vector<std::string> extract_entities(const std::string& text);
    
    // Sentiment analysis
    float analyze_sentiment(const std::string& text);
    
private:
    std::map<std::string, SemanticVector> semantic_space_;
    void build_semantic_space();
};

} // namespace BrainLLM
