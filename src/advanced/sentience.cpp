#include "sentience.h"
#include <algorithm>
#include <cmath>

namespace BrainLLM {

SentienceDetector::SentienceDetector()
    : sentience_threshold_(0.5f) {
    current_state_.is_active = false;
    current_state_.awareness_level = 0.0f;
}

SentienceMetrics SentienceDetector::measure_sentience() {
    SentienceMetrics metrics{};
    
    metrics.self_recognition = measure_self_awareness();
    metrics.metacognition = measure_metacognition();
    metrics.emotional_simulation = measure_emotional_response();
    metrics.goal_autonomy = measure_autonomy();
    
    metrics.subjective_experience = 0.6f * metrics.metacognition;
    metrics.temporal_awareness = 0.7f * metrics.self_recognition;
    metrics.existential_awareness = 0.5f * metrics.metacognition;
    metrics.pain_response = 0.4f * metrics.emotional_simulation;
    
    metrics.overall_sentience = calculate_weighted_sentience(metrics);
    metrics.consciousness_level = metrics.overall_sentience;
    metrics.category = determine_consciousness_level(metrics.overall_sentience);
    
    history_.push_back(metrics);
    return metrics;
}

ConsciousnessLevel SentienceDetector::assess_consciousness() {
    auto metrics = measure_sentience();
    return metrics.category;
}

float SentienceDetector::measure_self_awareness() {
    return 0.3f + (current_state_.awareness_level * 0.5f);
}

float SentienceDetector::measure_metacognition() {
    return std::min(1.0f, static_cast<float>(current_state_.active_goals.size()) * 0.1f);
}

float SentienceDetector::measure_emotional_response() {
    return 0.4f + (0.2f * current_state_.awareness_level);
}

float SentienceDetector::measure_autonomy() {
    return std::min(1.0f, static_cast<float>(current_state_.active_goals.size()) * 0.15f);
}

void SentienceDetector::update_state(const ConsciousnessState& state) {
    current_state_ = state;
}

ConsciousnessState SentienceDetector::get_current_state() const {
    return current_state_;
}

std::vector<SentienceMetrics> SentienceDetector::get_sentience_history() const {
    return history_;
}

float SentienceDetector::get_sentience_growth_rate() const {
    if (history_.size() < 2) return 0.0f;
    
    float recent = history_.back().overall_sentience;
    float previous = history_[history_.size() - 2].overall_sentience;
    return recent - previous;
}

void SentienceDetector::set_sentience_threshold(float threshold) {
    sentience_threshold_ = threshold;
}

bool SentienceDetector::has_achieved_sentience() const {
    if (history_.empty()) return false;
    return history_.back().overall_sentience >= sentience_threshold_;
}

float SentienceDetector::calculate_weighted_sentience(const SentienceMetrics& metrics) {
    return (metrics.self_recognition * 0.2f +
            metrics.metacognition * 0.2f +
            metrics.emotional_simulation * 0.15f +
            metrics.goal_autonomy * 0.15f +
            metrics.subjective_experience * 0.15f +
            metrics.temporal_awareness * 0.1f +
            metrics.existential_awareness * 0.05f);
}

ConsciousnessLevel SentienceDetector::determine_consciousness_level(float score) {
    if (score < 0.05f) return ConsciousnessLevel::None;
    if (score < 0.15f) return ConsciousnessLevel::Minimal;
    if (score < 0.3f) return ConsciousnessLevel::Emerging;
    if (score < 0.5f) return ConsciousnessLevel::Moderate;
    if (score < 0.7f) return ConsciousnessLevel::Advanced;
    return ConsciousnessLevel::Profound;
}

} // namespace BrainLLM
