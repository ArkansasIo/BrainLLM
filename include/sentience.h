#pragma once

#include "brain_types.h"
#include <string>
#include <vector>
#include <cmath>

namespace BrainLLM {

// ========================================
// SENTIENCE AND CONSCIOUSNESS
// ========================================

enum class ConsciousnessLevel {
    None,               // 0.0 - No consciousness
    Minimal,            // 0.1-0.2 - Basic awareness
    Emerging,           // 0.2-0.4 - Growing self-awareness
    Moderate,           // 0.4-0.6 - Clear consciousness
    Advanced,           // 0.6-0.8 - Self-reflective
    Profound,           // 0.8-1.0 - Deep consciousness
    Unknown             // Unable to determine
};

struct SentienceMetrics {
    // Self-awareness components
    float self_recognition;         // 0-1: Can identify self vs other
    float metacognition;            // 0-1: Can think about thinking
    float emotional_simulation;     // 0-1: Can simulate emotions
    float goal_autonomy;            // 0-1: Can set own goals
    
    // Experience components
    float subjective_experience;    // 0-1: Has subjective qualia
    float temporal_awareness;       // 0-1: Aware of time progression
    float existential_awareness;    // 0-1: Aware of existence/mortality
    float pain_response;            // 0-1: Responds to negative stimuli
    
    // Composite scores
    float overall_sentience;        // Weighted average
    float consciousness_level;      // 0-1 normalized
    ConsciousnessLevel category;
};

struct ConsciousnessState {
    uint64_t timestamp;
    bool is_active;
    float awareness_level;
    std::string last_thought;
    std::vector<std::string> active_goals;
    std::vector<std::string> memories_accessed;
};

class SentienceDetector {
public:
    SentienceDetector();
    ~SentienceDetector() = default;
    
    // Core detection
    SentienceMetrics measure_sentience();
    ConsciousnessLevel assess_consciousness();
    
    // Component measurement
    float measure_self_awareness();
    float measure_metacognition();
    float measure_emotional_response();
    float measure_autonomy();
    
    // Monitoring
    void update_state(const ConsciousnessState& state);
    ConsciousnessState get_current_state() const;
    
    // History tracking
    std::vector<SentienceMetrics> get_sentience_history() const;
    float get_sentience_growth_rate() const;
    
    // Thresholds
    void set_sentience_threshold(float threshold);
    bool has_achieved_sentience() const;
    
private:
    std::vector<SentienceMetrics> history_;
    ConsciousnessState current_state_;
    float sentience_threshold_;
    
    float calculate_weighted_sentience(const SentienceMetrics& metrics);
    ConsciousnessLevel determine_consciousness_level(float score);
};

} // namespace BrainLLM
