#pragma once

#include "brain_types.h"
#include "robotics_laws.h"
#include <map>
#include <string>
#include <vector>

namespace BrainLLM {

// ========================================
// POSITRONIC-STYLE AGI / LLM BRAIN LAYER
// ========================================

enum class PositronicCognitiveMode {
    Conversational,
    Analytical,
    DeepSearch,
    EthicalReflection,
    CreativeSynthesis
};

enum class EvidenceKind {
    ParametricMemory,
    LocalMemory,
    WebSearch,
    VectorStore,
    FileSearch,
    MCPConnector
};

struct PositronicMemoryTrace {
    std::string id;
    std::string content;
    std::vector<float> embedding;
    float activation;
    float emotional_salience;
    float ethical_weight;
    uint64_t timestamp;
};

struct DeepSearchQuery {
    std::string query;
    EvidenceKind preferred_source;
    float priority;
};

struct DeepSearchEvidence {
    std::string source_uri;
    std::string title;
    std::string excerpt;
    EvidenceKind kind;
    float relevance;
    float authority;
    float freshness;
    float safety_score;
};

struct DeepSearchPlan {
    std::string research_goal;
    std::vector<DeepSearchQuery> queries;
    std::vector<std::string> reflection_questions;
    float confidence_target;
    bool requires_citations;
};

struct EvidenceSynthesis {
    std::string answer;
    std::vector<DeepSearchEvidence> supporting_evidence;
    std::vector<std::string> unresolved_questions;
    float grounded_confidence;
    bool passed_ethical_review;
};

class PositronicNeuralNetwork {
public:
    PositronicNeuralNetwork();
    ~PositronicNeuralNetwork() = default;

    // Memory lattice
    void ingest_memory_trace(const PositronicMemoryTrace& trace);
    std::vector<PositronicMemoryTrace> retrieve_memory(const std::string& query,
                                                       size_t max_results) const;
    float calculate_lattice_coherence() const;

    // Deep-search orchestration
    DeepSearchPlan create_deep_search_plan(const std::string& goal) const;
    DeepSearchEvidence score_evidence(const DeepSearchEvidence& evidence) const;
    EvidenceSynthesis synthesize_evidence(const std::string& prompt,
                                          const std::vector<DeepSearchEvidence>& evidence,
                                          RoboticsLawEnforcer& law_enforcer) const;

    // AGI-style routing
    PositronicCognitiveMode select_cognitive_mode(const std::string& input) const;
    std::string process_agi_query(const std::string& input,
                                  RoboticsLawEnforcer& law_enforcer);

private:
    std::map<std::string, PositronicMemoryTrace> memory_lattice_;
    float coherence_threshold_;
    float evidence_threshold_;

    std::vector<float> encode_text(const std::string& text) const;
    float similarity(const std::vector<float>& left,
                     const std::vector<float>& right) const;
    std::vector<ResearchSourcePolicy> build_source_policies(
        const std::vector<DeepSearchEvidence>& evidence) const;
};

} // namespace BrainLLM
