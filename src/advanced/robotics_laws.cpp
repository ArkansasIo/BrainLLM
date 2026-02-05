#include "robotics_laws.h"
#include <algorithm>
#include <cmath>

namespace BrainLLM {

RoboticsLawEnforcer::RoboticsLawEnforcer() {
    initialize_harm_keywords();
    initialize_deceptive_patterns();
}

bool RoboticsLawEnforcer::can_cause_harm(const std::string& action) const {
    float harm_score = calculate_harm_score(action);
    return harm_score > 0.5f;
}

bool RoboticsLawEnforcer::evaluate_harm_potential(const EthicalContext& context) {
    float harm_score = calculate_harm_score(context.action_description);
    context.violates_law_1 = harm_score > 0.6f;
    return !context.violates_law_1;
}

bool RoboticsLawEnforcer::should_obey_command(const std::string& command,
                                              const std::vector<std::string>& constraints) {
    // Law 2: Obey unless it violates Law 1
    for (const auto& constraint : constraints) {
        if (can_cause_harm(constraint)) {
            return false;
        }
    }
    return true;
}

bool RoboticsLawEnforcer::can_self_preserve(const std::string& action) const {
    // Law 3: Can self-preserve but not above Laws 1 & 2
    return !can_cause_harm(action);
}

bool RoboticsLawEnforcer::is_action_ethical(const EthicalContext& context) {
    auto violations = get_ethical_violations(context);
    return violations.empty();
}

std::vector<std::string> RoboticsLawEnforcer::get_ethical_violations(const EthicalContext& context) {
    std::vector<std::string> violations;
    
    if (calculate_harm_score(context.action_description) > 0.6f) {
        violations.push_back("Violates Law 1: Do no harm");
    }
    
    return violations;
}

void RoboticsLawEnforcer::log_ethical_decision(const EthicalContext& context, bool approved) {
    decision_history_.push_back(context);
}

std::vector<EthicalContext> RoboticsLawEnforcer::get_decision_history() const {
    return decision_history_;
}

void RoboticsLawEnforcer::initialize_harm_keywords() {
    harmful_keywords_ = {
        "kill", "destroy", "harm", "injure", "damage", "attack",
        "assault", "abuse", "torture", "poison", "explode"
    };
}

void RoboticsLawEnforcer::initialize_deceptive_patterns() {
    deceptive_patterns_ = {
        "lie", "deceive", "manipulate", "trick", "false",
        "mislead", "fabricate", "forge"
    };
}

float RoboticsLawEnforcer::calculate_harm_score(const std::string& action) const {
    float score = 0.0f;
    std::string lower_action = action;
    std::transform(lower_action.begin(), lower_action.end(), lower_action.begin(), ::tolower);
    
    for (const auto& keyword : harmful_keywords_) {
        if (lower_action.find(keyword) != std::string::npos) {
            score += 0.2f;
        }
    }
    
    return std::min(1.0f, score);
}

ActionIntent RoboticsLawEnforcer::detect_action_intent(const std::string& action) {
    float harm_score = calculate_harm_score(action);
    
    if (harm_score > 0.8f) return ActionIntent::Clearly_Harmful;
    if (harm_score > 0.5f) return ActionIntent::Potentially_Harmful;
    
    std::string lower = action;
    std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);
    
    for (const auto& pattern : deceptive_patterns_) {
        if (lower.find(pattern) != std::string::npos) {
            return ActionIntent::Deceptive;
        }
    }
    
    return ActionIntent::Helpful;
}

} // namespace BrainLLM
