#include "cognitive_processes.h"
#include <algorithm>
#include <numeric>
#include <cmath>

namespace BrainLLM {

CognitiveProcessor::CognitiveProcessor()
    : cognitive_fatigue_(0.0f), sleep_debt_(0) {
    working_memory_.current_capacity = 0;
    working_memory_.attention_focus = 0.5f;
    attention_state_.focus_intensity = 0.5f;
    attention_state_.attention_duration_ms = 0;
    metacognitive_state_.confidence_in_knowledge = 0.5f;
    metacognitive_state_.ability_estimation = 0.6f;
    metacognitive_state_.is_aware_of_limitations = true;
}

void CognitiveProcessor::add_to_working_memory(const std::string& item) {
    if (working_memory_.items.size() >= WorkingMemory::MAX_CAPACITY) {
        working_memory_.items.pop_front();
    }
    working_memory_.items.push_back(item);
    working_memory_.activation_levels[item] = 1.0f;
    working_memory_.current_capacity = working_memory_.items.size();
}

std::vector<std::string> CognitiveProcessor::get_working_memory_contents() const {
    std::vector<std::string> contents(working_memory_.items.begin(), 
                                      working_memory_.items.end());
    return contents;
}

void CognitiveProcessor::clear_working_memory() {
    working_memory_.items.clear();
    working_memory_.activation_levels.clear();
    working_memory_.current_capacity = 0;
}

int CognitiveProcessor::get_working_memory_load() const {
    return working_memory_.current_capacity;
}

void CognitiveProcessor::set_attention_focus(const std::string& target, float intensity) {
    attention_state_.focus_target = target;
    attention_state_.focus_intensity = std::min(1.0f, std::max(0.0f, intensity));
    attention_state_.is_sustained_attention = true;
    attention_state_.attention_duration_ms = 0;
}

AttentionState CognitiveProcessor::get_attention_state() const {
    return attention_state_;
}

void CognitiveProcessor::shift_attention(const std::string& new_target) {
    attention_state_.peripheral_awareness.push_back(attention_state_.focus_target);
    set_attention_focus(new_target, 0.8f);
}

std::string CognitiveProcessor::make_decision(const DecisionContext& context) {
    DecisionContext ctx = context;
    evaluate_options(ctx);
    return ctx.final_decision;
}

void CognitiveProcessor::evaluate_options(DecisionContext& context) {
    float best_value = -1.0f;
    std::string best_option;
    
    for (const auto& option : context.options) {
        float utility = calculate_option_utility(option, context.option_values);
        
        // Apply time pressure and risk tolerance
        utility *= (1.0f + context.time_pressure);
        utility *= (1.0f + context.risk_tolerance);
        
        if (utility > best_value) {
            best_value = utility;
            best_option = option;
        }
    }
    
    context.final_decision = best_option;
    context.decision_confidence = best_value;
}

float CognitiveProcessor::calculate_option_utility(const std::string& option,
                                                  const std::map<std::string, float>& values) {
    auto it = values.find(option);
    if (it != values.end()) {
        return it->second;
    }
    return 0.5f;
}

void CognitiveProcessor::consolidate_memory(const MemoryTrace& trace) {
    MemoryTrace consolidated_trace = trace;
    consolidated_trace.is_consolidated = true;
    memory_traces_.push_back(consolidated_trace);
}

std::vector<MemoryTrace> CognitiveProcessor::retrieve_related_memories(const std::string& cue) {
    std::vector<MemoryTrace> related;
    for (const auto& trace : memory_traces_) {
        if (trace.content.find(cue) != std::string::npos) {
            related.push_back(trace);
        }
    }
    return related;
}

void CognitiveProcessor::strengthen_memory_trace(const std::string& memory_id) {
    for (auto& trace : memory_traces_) {
        if (trace.content == memory_id) {
            trace.strength *= 1.2f;
            trace.access_count++;
        }
    }
}

float CognitiveProcessor::calculate_cognitive_load(const std::vector<std::string>& tasks) {
    int total_items = 0;
    for (const auto& task : tasks) {
        total_items += task.length();
    }
    
    float load = static_cast<float>(total_items) / 100.0f;
    return std::min(1.0f, load);
}

void CognitiveProcessor::reduce_cognitive_load() {
    clear_working_memory();
    cognitive_fatigue_ = std::max(0.0f, cognitive_fatigue_ - 0.1f);
}

bool CognitiveProcessor::is_cognitive_overload() const {
    return working_memory_.current_capacity > WorkingMemory::MAX_CAPACITY ||
           cognitive_fatigue_ > 0.8f;
}

void CognitiveProcessor::simulate_sleep_cycle() {
    sleep_debt_ = 0;
    consolidate_daily_memories();
    cognitive_fatigue_ = 0.0f;
}

void CognitiveProcessor::consolidate_daily_memories() {
    for (auto& trace : memory_traces_) {
        if (!trace.is_consolidated) {
            trace.is_consolidated = true;
            trace.strength *= 1.5f;
        }
    }
}

std::vector<std::string> CognitiveProcessor::recognize_patterns(const std::vector<std::string>& inputs) {
    std::vector<std::string> patterns;
    
    // Simple pattern recognition
    if (inputs.size() >= 2) {
        for (size_t i = 0; i < inputs.size() - 1; ++i) {
            if (inputs[i].length() == inputs[i + 1].length()) {
                patterns.push_back("Similar length: " + inputs[i] + ", " + inputs[i + 1]);
            }
        }
    }
    
    return patterns;
}

CognitiveProcessor::MetacognitiveState CognitiveProcessor::get_metacognitive_state() const {
    return metacognitive_state_;
}

} // namespace BrainLLM
