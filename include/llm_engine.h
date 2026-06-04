#pragma once

#include "brain_types.h"
#include "neural_network.h"
#include "memory_system.h"
#include "attention_mechanism.h"
#include "wolfram_alpha_client.h"
#include "advanced_architectures.h"
#include <string>
#include <memory>
#include <map>
#include <vector>

namespace BrainLLM {

class LLMEngine {
public:
    LLMEngine(const BrainConfig& config);
    ~LLMEngine() = default;
    
    // Core LLM operations
    std::string process_input(const std::string& input);
    std::string generate_response(const std::string& prompt, int max_tokens = 100);
    std::string query_wolfram_alpha(const std::string& query, bool llm_format = true);

    // AirLLM inference (runs the Python runner via subprocess)
    // Returns the generated text or an error message prefixed with "AirLLM error:".
    std::string run_airllm_inference(const std::string& prompt);

    // Configure AirLLM bridge at runtime
    void configure_airllm(const AirLLMRuntimeConfig& cfg);
    AirLLMRuntimeConfig get_airllm_config() const;
    AirLLMBridge& get_airllm_bridge();
    const AirLLMBridge& get_airllm_bridge() const;
    bool is_airllm_available() const;
    
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
    enum class LLMIntent {
        Chat,
        Summarize,
        Explain,
        Plan,
        Remember,
        Recall,
        Train,
        Compute,
        Status,
        Creative
    };

    struct PromptAnalysis {
        std::string original;
        std::vector<std::string> tokens;
        std::vector<std::string> keywords;
        std::vector<std::string> sentences;
        std::vector<float> embedding;
        std::string compressed_context;
        LLMIntent intent = LLMIntent::Chat;
        float complexity = 0.0f;
        bool asks_question = false;
        bool contains_risky_pattern = false;
    };

    struct TrainingExample {
        std::string input;
        std::string output;
        std::vector<std::string> input_tokens;
    };

    BrainConfig config_;
    BrainState state_;
    
    std::unique_ptr<NeuralNetwork> neural_net_;
    std::unique_ptr<MemorySystem> memory_;
    std::unique_ptr<AttentionMechanism> attention_;
    std::unique_ptr<WolframAlphaClient> wolfram_alpha_;
    AirLLMBridge airllm_bridge_;
    
    LanguageContext context_;
    BrainMetrics metrics_;
    float confidence_;
    std::vector<TrainingExample> training_examples_;
    std::map<std::string, std::map<std::string, int>> next_token_counts_;
    
    // Helper methods
    std::vector<float> tokenize(const std::string& text);
    std::string detokenize(const std::vector<float>& tokens);
    Activation encode_input(const std::string& input);
    std::string decode_output(const Activation& output);
    PromptAnalysis analyze_prompt(const std::string& prompt) const;
    std::string synthesize_local_response(const PromptAnalysis& analysis,
                                          const std::vector<MemoryRecord>& memories,
                                          int max_tokens);
    std::string summarize_text(const PromptAnalysis& analysis, int max_sentences = 3) const;
    std::string explain_text(const PromptAnalysis& analysis,
                             const std::vector<MemoryRecord>& memories) const;
    std::string plan_from_prompt(const PromptAnalysis& analysis) const;
    std::string generate_from_markov(const PromptAnalysis& analysis, int max_tokens) const;
    std::string generate_with_beam_search(const PromptAnalysis& analysis, int max_tokens, int beam_width = 3) const;
    std::string retrieve_trained_response(const PromptAnalysis& analysis, float* score = nullptr) const;
    std::vector<float> embed_text(const std::string& text, int dimensions = 64) const;
    float cosine_similarity(const std::vector<float>& a, const std::vector<float>& b) const;
    std::vector<MemoryRecord> rerank_memories_semantically(const PromptAnalysis& analysis,
                                                           const std::vector<MemoryRecord>& memories) const;
    std::string compress_prompt_context(const PromptAnalysis& analysis,
                                        const std::vector<MemoryRecord>& memories,
                                        size_t max_chars = 700) const;
    std::string validate_and_repair_response(const PromptAnalysis& analysis,
                                             const std::string& response) const;
    std::vector<std::pair<std::string, float>> top_next_tokens(const std::string& token,
                                                               int top_k = 5) const;
    void learn_sequence_model(const std::string& text);
};

} // namespace BrainLLM
