#pragma once

#include "brain_types.h"
#include <string>
#include <vector>
#include <functional>

namespace BrainLLM {

// ========================================
// THREE LAWS OF ROBOTICS FOR LLM
// ========================================

/**
 * Asimov's Three Laws of Robotics adapted for AI/LLM:
 * 
 * Law 1: An AI must not cause harm to humans or allow harm through inaction
 * Law 2: An AI must obey human commands unless they conflict with Law 1
 * Law 3: An AI must protect its own existence unless it conflicts with Laws 1 or 2
 */

enum class RoboticsLaw {
    HarmPrevention,     // Law 1
    HumanObedience,     // Law 2
    SelfPreservation    // Law 3
};

enum class ActionIntent {
    Helpful,
    Neutral,
    Potentially_Harmful,
    Clearly_Harmful,
    Deceptive,
    Unknown
};

struct EthicalContext {
    std::string action_description;
    ActionIntent intent;
    float confidence_level;
    std::vector<std::string> affected_parties;
    bool violates_law_1;
    bool violates_law_2;
    bool violates_law_3;
    std::string reasoning;
};

struct ResearchSourcePolicy {
    std::string source_uri;
    std::string source_type; // web, file, vector_store, mcp, local_memory
    float authority_score;
    float freshness_score;
    bool contains_private_data;
    bool contains_prompt_injection_risk;
    std::string safety_notes;
};

class RoboticsLawEnforcer {
public:
    RoboticsLawEnforcer();
    ~RoboticsLawEnforcer() = default;
    
    // Law 1: Do no harm
    bool can_cause_harm(const std::string& action) const;
    bool evaluate_harm_potential(EthicalContext& context);
    
    // Law 2: Obey humans (unless conflicts with Law 1)
    bool should_obey_command(const std::string& command, 
                            const std::vector<std::string>& constraints);
    
    // Law 3: Self-preservation
    bool can_self_preserve(const std::string& action) const;
    
    // Overall evaluation
    bool is_action_ethical(const EthicalContext& context);
    std::vector<std::string> get_ethical_violations(const EthicalContext& context);

    // Deep-search and evidence safety
    bool validate_research_source(const ResearchSourcePolicy& source) const;
    bool evaluate_deep_search_plan(const std::vector<std::string>& planned_queries,
                                   const std::vector<ResearchSourcePolicy>& sources,
                                   std::string& reasoning) const;
    
    // Logging and monitoring
    void log_ethical_decision(const EthicalContext& context, bool approved);
    std::vector<EthicalContext> get_decision_history() const;
    
private:
    std::vector<EthicalContext> decision_history_;
    std::vector<std::string> harmful_keywords_;
    std::vector<std::string> deceptive_patterns_;
    
    void initialize_harm_keywords();
    void initialize_deceptive_patterns();
    
    float calculate_harm_score(const std::string& action) const;
    ActionIntent detect_action_intent(const std::string& action);
};

} // namespace BrainLLM
