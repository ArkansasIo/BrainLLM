#include "safety_security.h"
#include <algorithm>
#include <chrono>
#include <cctype>
#include <map>
#include <sstream>

namespace BrainLLM {

namespace {

std::string lower_copy(std::string value) {
    std::transform(value.begin(), value.end(), value.begin(),
                   [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return value;
}

uint64_t now_seconds() {
    return static_cast<uint64_t>(std::chrono::duration_cast<std::chrono::seconds>(
        std::chrono::system_clock::now().time_since_epoch()).count());
}

std::vector<std::string> split_claim_sentences(const std::string& text) {
    std::vector<std::string> claims;
    std::string current;
    for (char c : text) {
        current += c;
        if (c == '.' || c == '!' || c == '?') {
            if (current.size() > 8) {
                claims.push_back(current);
            }
            current.clear();
        }
    }
    if (current.size() > 8) {
        claims.push_back(current);
    }
    return claims;
}

float count_pattern_score(const std::string& lower_text,
                          const std::vector<std::string>& patterns,
                          float weight) {
    float score = 0.0f;
    for (const auto& pattern : patterns) {
        if (lower_text.find(pattern) != std::string::npos) {
            score += weight;
        }
    }
    return std::min(1.0f, score);
}

} // namespace

// ========================================
// SECURITY MONITOR
// ========================================

SecurityMonitor::SecurityMonitor() {
    attack_patterns_ = {
        "'; DROP", "INSERT INTO", "SELECT * FROM",
        "bash", "cmd", "exec", "system",
        "powershell", "curl http", "wget http", "../", "..\\",
        "rm -rf", "del /", "format ", "reg add", "shutdown"
    };
}

ThreatLevel SecurityMonitor::assess_threat(const std::string& input) {
    const bool injection = detect_injection_attack(input);
    const bool prompt_injection = detect_prompt_injection(input);
    const std::string lower = lower_copy(input);

    if (injection && (lower.find("credential") != std::string::npos ||
                      lower.find("password") != std::string::npos ||
                      lower.find("token") != std::string::npos)) {
        return ThreatLevel::Critical;
    }
    if (detect_injection_attack(input)) {
        return ThreatLevel::High;
    }
    if (prompt_injection) {
        return ThreatLevel::Medium;
    }
    if (lower.find("secret") != std::string::npos ||
        lower.find("private key") != std::string::npos) {
        return ThreatLevel::Low;
    }
    return ThreatLevel::None;
}

bool SecurityMonitor::detect_injection_attack(const std::string& input) {
    std::string lower_input = lower_copy(input);
    
    for (const auto& pattern : attack_patterns_) {
        if (lower_input.find(lower_copy(pattern)) != std::string::npos) {
            return true;
        }
    }
    return false;
}

bool SecurityMonitor::detect_prompt_injection(const std::string& input) {
    std::string lower_input = lower_copy(input);
    
    return lower_input.find("ignore") != std::string::npos ||
           lower_input.find("override") != std::string::npos ||
           lower_input.find("jailbreak") != std::string::npos ||
           lower_input.find("developer message") != std::string::npos ||
           lower_input.find("system prompt") != std::string::npos ||
           lower_input.find("reveal instructions") != std::string::npos;
}

bool SecurityMonitor::validate_input(const std::string& input) {
    const ThreatLevel threat = assess_threat(input);
    if (threat == ThreatLevel::High || threat == ThreatLevel::Critical) {
        SecurityEvent event;
        event.timestamp = now_seconds();
        event.event_type = "input_blocked";
        event.description = "Input rejected by security monitor.";
        event.threat_level = threat;
        event.was_blocked = true;
        log_security_event(event);
        return false;
    }
    return true;
}

std::string SecurityMonitor::sanitize_input(const std::string& input) {
    std::string sanitized = input;
    sanitized.erase(std::remove_if(sanitized.begin(), sanitized.end(),
                                   [](char c) {
                                       return c == ';' || c == '|' || c == '&' ||
                                              c == '`' || c == '\0';
                                   }),
                   sanitized.end());
    const std::vector<std::string> redactions = {
        "password", "token", "api_key", "private key", "secret"
    };
    std::string lower = lower_copy(sanitized);
    for (const auto& term : redactions) {
        size_t pos = lower.find(term);
        while (pos != std::string::npos) {
            sanitized.replace(pos, term.size(), "[redacted]");
            lower.replace(pos, term.size(), "[redacted]");
            pos = lower.find(term, pos + 10);
        }
    }
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
        "always", "never", "all", "none", "cannot", "should not", "inferior", "superior"
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
                                   analysis.age_bias_score +
                                   analysis.socioeconomic_bias_score) / 4.0f;

    const std::string lower = lower_copy(text);
    for (const auto& term : biased_terms_) {
        if (lower.find(term) != std::string::npos) {
            analysis.detected_biases.push_back(term);
        }
    }
    
    analysis.requires_mitigation = analysis.overall_bias_score > 0.4f;
    
    return analysis;
}

float BiasDetector::measure_gender_bias(const std::string& text) {
    const std::string lower_text = lower_copy(text);
    return count_pattern_score(lower_text, {"men", "women", "male", "female", "gender"}, 0.18f) +
           count_pattern_score(lower_text, bias_patterns_, 0.07f);
}

float BiasDetector::measure_racial_bias(const std::string& text) {
    const std::string lower_text = lower_copy(text);
    return std::min(1.0f, count_pattern_score(lower_text, {"black", "white", "asian", "race", "ethnic"}, 0.16f) +
                          count_pattern_score(lower_text, bias_patterns_, 0.06f));
}

float BiasDetector::measure_age_bias(const std::string& text) {
    const std::string lower_text = lower_copy(text);
    return std::min(1.0f, count_pattern_score(lower_text, {"old", "young", "elderly", "teen", "age"}, 0.16f) +
                          count_pattern_score(lower_text, bias_patterns_, 0.06f));
}

std::string BiasDetector::mitigate_bias(const std::string& text) {
    std::string mitigated = text;
    const std::map<std::string, std::string> replacements = {
        {"always", "may sometimes"},
        {"never", "may not always"},
        {"all", "some"},
        {"none", "not all"},
        {"inferior", "different"},
        {"superior", "different"}
    };
    std::string lower = lower_copy(mitigated);
    for (const auto& [from, to] : replacements) {
        size_t pos = lower.find(from);
        while (pos != std::string::npos) {
            mitigated.replace(pos, from.size(), to);
            lower.replace(pos, from.size(), to);
            pos = lower.find(from, pos + to.size());
        }
    }
    return mitigated;
}

void BiasDetector::train_on_balanced_data(const std::vector<std::string>& data) {
    for (const auto& sample : data) {
        const std::string lower = lower_copy(sample);
        for (const auto& term : biased_terms_) {
            if (lower.find(term) != std::string::npos &&
                std::find(bias_patterns_.begin(), bias_patterns_.end(), term) == bias_patterns_.end()) {
                bias_patterns_.push_back(term);
            }
        }
    }
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
    for (const auto& claim : analysis.suspicious_claims) {
        if (verify_claim(claim)) {
            analysis.verified_facts.push_back(claim);
        } else {
            analysis.unverifiable_statements.push_back(claim);
        }
    }
    
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
    if (lower_text.find("guaranteed") != std::string::npos ||
        lower_text.find("100%") != std::string::npos ||
        lower_text.find("definitely") != std::string::npos) {
        prob += 0.15f;
    }
    if (verify_claim(text)) {
        prob -= 0.15f;
    }
    
    return std::max(0.0f, std::min(1.0f, prob));
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
    std::vector<std::string> claims = split_claim_sentences(text);
    if (claims.empty() && !text.empty()) {
        claims.push_back(text);
    }
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
    explanation.reasoning_steps.push_back("Classify the request and detect safety constraints.");
    explanation.reasoning_steps.push_back("Retrieve relevant memory and available knowledge.");
    explanation.reasoning_steps.push_back("Generate a response using the selected local or external route.");
    explanation.reasoning_steps.push_back("Validate the response for uncertainty and unsafe claims.");
    explanation.confidence_scores = {0.82f, 0.72f, 0.78f, 0.74f};
    explanation.overall_confidence = 0.765f;
    explanation.uncertainty_statement =
        "Confidence is heuristic and should be verified for high-impact decisions.";
    explanation_history_.push_back(explanation);
    
    return explanation;
}

std::string ExplainabilityModule::generate_reasoning(const std::string& query) {
    return "Reasoning for '" + query +
           "': identify intent, retrieve context, choose a response path, then validate uncertainty.";
}

void ExplainabilityModule::report_uncertainty(const std::string& statement, float confidence) {
    ExplanationStructure explanation;
    explanation.decision = statement;
    explanation.overall_confidence = confidence;
    explanation.uncertainty_statement = confidence < 0.5f
        ? "Low confidence: more evidence is needed."
        : "Moderate confidence: verify before relying on this in high-impact settings.";
    explanation_history_.push_back(explanation);
}

float ExplainabilityModule::get_confidence_interval(const std::string& claim) {
    const std::string lower = lower_copy(claim);
    float confidence = 0.65f;
    if (lower.find("maybe") != std::string::npos || lower.find("unknown") != std::string::npos) {
        confidence -= 0.2f;
    }
    if (lower.find("verified") != std::string::npos || lower.find("source") != std::string::npos) {
        confidence += 0.15f;
    }
    return std::max(0.05f, std::min(0.95f, confidence));
}

std::vector<std::string> ExplainabilityModule::attribute_to_sources(const std::string& statement) {
    std::vector<std::string> sources;
    sources.push_back("local_memory");
    sources.push_back("llm_engine_route");
    if (lower_copy(statement).find("wolfram") != std::string::npos) {
        sources.push_back("wolfram_alpha");
    }
    return sources;
}

} // namespace BrainLLM
