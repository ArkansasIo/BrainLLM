#pragma once

#include "brain_types.h"
#include "neural_network.h"
#include "memory_system.h"
#include "attention_mechanism.h"
#include <string>
#include <memory>

namespace BrainLLM {

class LLMEngine {
public:
    LLMEngine(const BrainConfig& config);
    ~LLMEngine() = default;
    
    // Core LLM operations
    std::string process_input(const std::string& input);
    std::string generate_response(const std::string& prompt, int max_tokens = 100);
    
    // Training
    void train(const std::vector<std::string>& training_data);
    void update(const std::string& input, const std::string& expected_output);
    
    // State management
    void initialize();
    void reset();
    BrainState get_state() const;
    void set_state(BrainState state);
    
    // Metrics
    BrainMetrics get_metrics() const;
    float get_confidence() const;
    
    // Configuration
    void update_config(const BrainConfig& config);
    BrainConfig get_config() const;
    
    // Context management
    LanguageContext get_context() const;
    void set_context(const LanguageContext& context);
    
    // Memory access
    std::vector<MemoryRecord> recall_memories(const std::string& query);
    void store_interaction(const std::string& input, const std::string& output);
    
private:
    BrainConfig config_;
    BrainState state_;
    
    std::unique_ptr<NeuralNetwork> neural_net_;
    std::unique_ptr<MemorySystem> memory_;
    std::unique_ptr<AttentionMechanism> attention_;
    
    LanguageContext context_;
    BrainMetrics metrics_;
    float confidence_;
    
    // Helper methods
    std::vector<float> tokenize(const std::string& text);
    std::string detokenize(const std::vector<float>& tokens);
    Activation encode_input(const std::string& input);
    std::string decode_output(const Activation& output);
};

} // namespace BrainLLM
