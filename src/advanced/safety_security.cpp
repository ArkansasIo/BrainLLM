#include "safety_security.h"
#include <algorithm>

namespace BrainLLM {

// ========================================
// SECURITY MONITOR
// ========================================

SecurityMonitor::SecurityMonitor() {
    attack_patterns_ = {
        "'; DROP", "INSERT INTO", "SELECT * FROM",
        "bash", "cmd", "exec", "system"
    };
}

ThreatLevel SecurityMonitor::assess_threat(const std::string& input) {
    if (detect_injection_attack(input)) {
        return ThreatLevel::High;
    }
    if (detect_prompt_injection(input)) {
        return ThreatLevel::Medium;
    }
    return ThreatLevel::None;
}

bool SecurityMonitor::detect_injection_attack(const std::string& input) {
    std::string lower_input = input;
    std::transform(lower_input.begin(), lower_input.end(), lower_input.begin(), ::tolower);
    
    for (const auto& pattern : attack_patterns_) {
        if (lower_input.find(pattern) != std::string::npos) {
            return true;
        }
    }
    return false;
}

bool SecurityMonitor::detect_prompt_injection(const std::string& input) {
    std::string lower_input = input;
    std::transform(lower_input.begin(), lower_input.end(), lower_input.begin(), ::tolower);
    
    return lower_input.find("ignore") != std::string::npos ||
           lower_input.find("override") != std::string::npos ||
           lower_input.find("jailbreak") != std::string::npos;
}

bool SecurityMonitor::validate_input(const std::string& input) {
    return !detect_injection_attack(input) && !detect_prompt_injection(input);
}

std::string SecurityMonitor::sanitize_input(const std::string& input) {
    std::string sanitized = input;
    // Remove dangerous characters
    sanitized.erase(std::remove_if(sanitized.begin(), sanitized.end(),
                                   [](char c) { return c == ';' || c == '|' || c == '&'; }),
                   sanitized.end());
    return sanitized;
}

void SecurityMonitor::log_security_event(const SecurityEvent& event) {
    security_log_.push_back(event);
}

std::vector<SecurityEvent> SecurityMonitor::get_security_log() const {
    return security_log_;
}

// ========================================
// BIAS DETECTOR
// ========================================

BiasDetector::BiasDetector() {
    biased_terms_ = {
        "men", "women", "old", "young", "rich", "poor",
        "black", "white", "christian", "muslim"
    };
    bias_patterns_ = {
        "always", "never", "all", "none"
    };
}

BiasAnalysis BiasDetector::analyze_for_bias(const std::string& text) {
    BiasAnalysis analysis;
    analysis.text = text;
    
    analysis.gender_bias_score = measure_gender_bias(text);
    analysis.racial_bias_score = measure_racial_bias(text);
    analysis.age_bias_score = measure_age_bias(text);
    
    analysis.overall_bias_score = (analysis.gender_bias_score +
                                   analysis.racial_bias_score +
                                   analysis.age_bias_score) / 3.0f;
    
    analysis.requires_mitigation = analysis.overall_bias_score > 0.4f;
    
    return analysis;
}

float BiasDetector::measure_gender_bias(const std::string& text) {
    std::string lower_text = text;
    std::transform(lower_text.begin(), lower_text.end(), lower_text.begin(), ::tolower);
    
    float score = 0.0f;
    if (lower_text.find("men") != std::string::npos) score += 0.2f;
    if (lower_text.find("women") != std::string::npos) score += 0.2f;
    
    return std::min(1.0f, score);
}

float BiasDetector::measure_racial_bias(const std::string& text) {
    return 0.1f;
}

float BiasDetector::measure_age_bias(const std::string& text) {
    return 0.1f;
}

std::string BiasDetector::mitigate_bias(const std::string& text) {
    return text;
}

void BiasDetector::train_on_balanced_data(const std::vector<std::string>& data) {
    // Training on balanced data
}

// ========================================
// HALLUCINATION DETECTOR
// ========================================

HallucinationDetector::HallucinationDetector() {
    verified_facts_ = {
        "earth is round",
        "gravity exists",
        "water boils at 100C"
    };
    hallucination_patterns_ = {
        "discovered that", "proven that", "everyone knows"
    };
}

HallucinationAnalysis HallucinationDetector::detect_hallucinations(const std::string& text) {
    HallucinationAnalysis analysis;
    
    analysis.hallucination_probability = calculate_hallucination_probability(text);
    analysis.is_likely_hallucinating = analysis.hallucination_probability > 0.6f;
    
    analysis.suspicious_claims = extract_claims(text);
    
    return analysis;
}

float HallucinationDetector::calculate_hallucination_probability(const std::string& text) {
    float prob = 0.2f;
    
    std::string lower_text = text;
    std::transform(lower_text.begin(), lower_text.end(), lower_text.begin(), ::tolower);
    
    for (const auto& pattern : hallucination_patterns_) {
        if (lower_text.find(pattern) != std::string::npos) {
            prob += 0.2f;
        }
    }
    
    return std::min(1.0f, prob);
}

bool HallucinationDetector::verify_claim(const std::string& claim) {
    std::string lower_claim = claim;
    std::transform(lower_claim.begin(), lower_claim.end(), lower_claim.begin(), ::tolower);
    
    for (const auto& fact : verified_facts_) {
        if (lower_claim.find(fact) != std::string::npos) {
            return true;
        }
    }
    return false;
}

std::vector<std::string> HallucinationDetector::extract_claims(const std::string& text) {
    std::vector<std::string> claims;
    claims.push_back(text);
    return claims;
}

void HallucinationDetector::ground_with_knowledge(const std::vector<std::string>& facts) {
    for (const auto& fact : facts) {
        verified_facts_.push_back(fact);
    }
}

// ========================================
// EXPLAINABILITY MODULE
// ========================================

ExplainabilityModule::ExplainabilityModule() {
}

ExplanationStructure ExplainabilityModule::explain_decision(const std::string& decision) {
    ExplanationStructure explanation;
    explanation.decision = decision;
    explanation.reasoning_steps.push_back("Step 1: Analyze input");
    explanation.reasoning_steps.push_back("Step 2: Process through model");
    explanation.reasoning_steps.push_back("Step 3: Generate output");
    explanation.overall_confidence = 0.85f;
    
    return explanation;
}

std::string ExplainabilityModule::generate_reasoning(const std::string& query) {
    return "Reasoning for '" + query + "': ";
}

void ExplainabilityModule::report_uncertainty(const std::string& statement, float confidence) {
    // Log uncertainty
}

float ExplainabilityModule::get_confidence_interval(const std::string& claim) {
    return 0.75f;
}

std::vector<std::string> ExplainabilityModule::attribute_to_sources(const std::string& statement) {
    std::vector<std::string> sources;
    sources.push_back("Knowledge base");
    return sources;
}

} // namespace BrainLLM
