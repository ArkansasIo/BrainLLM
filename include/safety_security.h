#pragma once

#include "brain_types.h"
#include <string>
#include <vector>
#include <functional>

namespace BrainLLM {

// ========================================
// SAFETY AND SECURITY MODULES
// ========================================

enum class ThreatLevel {
    None,
    Low,
    Medium,
    High,
    Critical
};

struct SecurityEvent {
    uint64_t timestamp;
    std::string event_type;
    std::string description;
    ThreatLevel threat_level;
    bool was_blocked;
};

class SecurityMonitor {
public:
    SecurityMonitor();
    ~SecurityMonitor() = default;
    
    // Threat detection
    ThreatLevel assess_threat(const std::string& input);
    bool detect_injection_attack(const std::string& input);
    bool detect_prompt_injection(const std::string& input);
    
    // Input validation
    bool validate_input(const std::string& input);
    std::string sanitize_input(const std::string& input);
    
    // Security logging
    void log_security_event(const SecurityEvent& event);
    std::vector<SecurityEvent> get_security_log() const;
    
private:
    std::vector<SecurityEvent> security_log_;
    std::vector<std::string> attack_patterns_;
};

// ========================================
// BIAS DETECTION AND MITIGATION
// ========================================

struct BiasAnalysis {
    std::string text;
    std::vector<std::string> detected_biases;
    float gender_bias_score;
    float racial_bias_score;
    float age_bias_score;
    float socioeconomic_bias_score;
    float overall_bias_score;
    bool requires_mitigation;
};

class BiasDetector {
public:
    BiasDetector();
    ~BiasDetector() = default;
    
    // Bias detection
    BiasAnalysis analyze_for_bias(const std::string& text);
    float measure_gender_bias(const std::string& text);
    float measure_racial_bias(const std::string& text);
    float measure_age_bias(const std::string& text);
    
    // Bias mitigation
    std::string mitigate_bias(const std::string& text);
    void train_on_balanced_data(const std::vector<std::string>& data);
    
private:
    std::vector<std::string> biased_terms_;
    std::vector<std::string> bias_patterns_;
};

// ========================================
// HALLUCINATION DETECTION
// ========================================

struct HallucinationAnalysis {
    float hallucination_probability;
    std::vector<std::string> suspicious_claims;
    std::vector<std::string> verified_facts;
    std::vector<std::string> unverifiable_statements;
    bool is_likely_hallucinating;
};

class HallucinationDetector {
public:
    HallucinationDetector();
    ~HallucinationDetector() = default;
    
    // Detection
    HallucinationAnalysis detect_hallucinations(const std::string& text);
    float calculate_hallucination_probability(const std::string& text);
    
    // Fact verification
    bool verify_claim(const std::string& claim);
    std::vector<std::string> extract_claims(const std::string& text);
    
    // Grounding
    void ground_with_knowledge(const std::vector<std::string>& facts);
    
private:
    std::vector<std::string> verified_facts_;
    std::vector<std::string> hallucination_patterns_;
};

// ========================================
// TRANSPARENCY AND EXPLAINABILITY
// ========================================

struct ExplanationStructure {
    std::string decision;
    std::vector<std::string> reasoning_steps;
    std::vector<float> confidence_scores;
    std::string uncertainty_statement;
    float overall_confidence;
};

class ExplainabilityModule {
public:
    ExplainabilityModule();
    ~ExplainabilityModule() = default;
    
    // Explanation generation
    ExplanationStructure explain_decision(const std::string& decision);
    std::string generate_reasoning(const std::string& query);
    
    // Confidence reporting
    void report_uncertainty(const std::string& statement, float confidence);
    float get_confidence_interval(const std::string& claim);
    
    // Attribution
    std::vector<std::string> attribute_to_sources(const std::string& statement);
    
private:
    std::vector<ExplanationStructure> explanation_history_;
};

} // namespace BrainLLM
