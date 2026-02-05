#pragma once

#include "brain_types.h"
#include <string>
#include <vector>
#include <map>
#include <deque>

namespace BrainLLM {

// ========================================
// HUMAN BRAIN COGNITIVE PROCESSES
// ========================================

/**
 * Simulates major cognitive processes in the human brain:
 * - Executive function and planning
 * - Working memory
 * - Long-term memory consolidation
 * - Decision making
 * - Attention and focus
 * - Pattern recognition
 */

enum class CognitiveProcess {
    Attention,          // Focus and selective awareness
    Perception,         // Sensory processing
    Memory,             // Encoding and retrieval
    Thinking,           // Reasoning and problem solving
    Language,           // Speech and comprehension
    Emotion,            // Affective processing
    Motor,              // Movement and action
    Sleep,              // Rest and consolidation
    Motivation          // Drive and goal-seeking
};

struct WorkingMemory {
    static const int MAX_CAPACITY = 7;  // Miller's Law: 7±2 items
    
    std::deque<std::string> items;
    std::map<std::string, float> activation_levels;
    int current_capacity;
    float attention_focus;
    std::vector<std::string> active_chunks;
};

struct AttentionState {
    std::string focus_target;
    float focus_intensity;
    std::vector<std::string> peripheral_awareness;
    bool is_sustained_attention;
    int attention_duration_ms;
    float mental_effort;
};

struct DecisionContext {
    std::vector<std::string> options;
    std::map<std::string, float> option_values;
    std::vector<std::string> constraints;
    float risk_tolerance;
    float time_pressure;
    std::string final_decision;
    float decision_confidence;
};

struct MemoryTrace {
    uint64_t timestamp;
    std::string content;
    std::string context;
    std::vector<std::string> associated_memories;
    float strength;
    int access_count;
    bool is_consolidated;
};

class CognitiveProcessor {
public:
    CognitiveProcessor();
    ~CognitiveProcessor() = default;
    
    // Working memory operations
    void add_to_working_memory(const std::string& item);
    std::vector<std::string> get_working_memory_contents() const;
    void clear_working_memory();
    int get_working_memory_load() const;
    
    // Attention control
    void set_attention_focus(const std::string& target, float intensity = 1.0f);
    AttentionState get_attention_state() const;
    void shift_attention(const std::string& new_target);
    
    // Decision making
    std::string make_decision(const DecisionContext& context);
    void evaluate_options(DecisionContext& context);
    float calculate_option_utility(const std::string& option, 
                                   const std::map<std::string, float>& values);
    
    // Memory operations
    void consolidate_memory(const MemoryTrace& trace);
    std::vector<MemoryTrace> retrieve_related_memories(const std::string& cue);
    void strengthen_memory_trace(const std::string& memory_id);
    
    // Cognitive load management
    float calculate_cognitive_load(const std::vector<std::string>& tasks);
    void reduce_cognitive_load();
    bool is_cognitive_overload() const;
    
    // Sleep and consolidation
    void simulate_sleep_cycle();
    void consolidate_daily_memories();
    
    // Pattern recognition
    std::vector<std::string> recognize_patterns(const std::vector<std::string>& inputs);
    
    // Metacognition (thinking about thinking)
    struct MetacognitiveState {
        float confidence_in_knowledge;
        float ability_estimation;
        std::vector<std::string> strategy_used;
        bool is_aware_of_limitations;
    };
    
    MetacognitiveState get_metacognitive_state() const;
    
private:
    WorkingMemory working_memory_;
    AttentionState attention_state_;
    std::vector<MemoryTrace> memory_traces_;
    std::map<std::string, float> cognitive_resources_;
    MetacognitiveState metacognitive_state_;
    
    float cognitive_fatigue_;
    int sleep_debt_;
};

} // namespace BrainLLM
