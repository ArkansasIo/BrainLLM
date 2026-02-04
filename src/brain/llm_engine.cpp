#include "llm_engine.h"
#include <algorithm>
#include <sstream>

namespace BrainLLM {

LLMEngine::LLMEngine(const BrainConfig& config)
    : config_(config), state_(BrainState::Idle), confidence_(0.0f) {
    neural_net_ = std::make_unique<NeuralNetwork>(config);
    memory_ = std::make_unique<MemorySystem>(config.max_memory_size);
    attention_ = std::make_unique<AttentionMechanism>(config.num_attention_heads, config.embedding_dim);
}

std::string LLMEngine::process_input(const std::string& input) {
    state_ = BrainState::Processing;
    
    context_.current_input = input;
    auto encoded = encode_input(input);
    auto output = neural_net_->forward(encoded);
    auto response = decode_output(output);
    
    context_.last_output = response;
    confidence_ = 0.75f;
    
    store_interaction(input, response);
    
    state_ = BrainState::Idle;
    return response;
}

std::string LLMEngine::generate_response(const std::string& prompt, int max_tokens) {
    state_ = BrainState::Processing;
    
    std::string response = "AI Response: ";
    
    // Retrieve relevant memories
    auto memories = memory_->retrieve_memories(prompt, 3);
    if (!memories.empty()) {
        response += "Based on memory: " + memories[0].content + ". ";
    }
    
    // Generate tokens
    for (int i = 0; i < max_tokens; ++i) {
        auto encoded = encode_input(prompt + response);
        auto output = neural_net_->forward(encoded);
        
        float max_val = -1.0f;
        size_t max_idx = 0;
        for (size_t j = 0; j < output.size(); ++j) {
            if (output[j] > max_val) {
                max_val = output[j];
                max_idx = j;
            }
        }
        
        response += char(32 + (max_idx % 94));
        if (max_val < 0.3f) break;
    }
    
    state_ = BrainState::Idle;
    return response;
}

void LLMEngine::train(const std::vector<std::string>& training_data) {
    state_ = BrainState::Learning;
    
    for (const auto& data : training_data) {
        auto encoded = encode_input(data);
        neural_net_->forward(encoded);
        neural_net_->backward(encoded);
        neural_net_->update_weights(config_.learning_rate);
    }
    
    state_ = BrainState::Idle;
}

void LLMEngine::update(const std::string& input, const std::string& expected_output) {
    auto encoded_input = encode_input(input);
    auto encoded_expected = encode_input(expected_output);
    
    neural_net_->forward(encoded_input);
    neural_net_->backward(encoded_expected);
    neural_net_->update_weights(config_.learning_rate);
    
    memory_->store_memory(input + " -> " + expected_output, 0.9f);
}

void LLMEngine::initialize() {
    state_ = BrainState::Processing;
    neural_net_->initialize_weights();
    memory_->clear_memories();
    state_ = BrainState::Idle;
}

void LLMEngine::reset() {
    state_ = BrainState::Idle;
    neural_net_->reset();
    memory_->clear_memories();
    context_ = LanguageContext();
    confidence_ = 0.0f;
}

BrainState LLMEngine::get_state() const {
    return state_;
}

void LLMEngine::set_state(BrainState state) {
    state_ = state;
}

BrainMetrics LLMEngine::get_metrics() const {
    return neural_net_->get_metrics();
}

float LLMEngine::get_confidence() const {
    return confidence_;
}

void LLMEngine::update_config(const BrainConfig& config) {
    config_ = config;
}

BrainConfig LLMEngine::get_config() const {
    return config_;
}

LanguageContext LLMEngine::get_context() const {
    return context_;
}

void LLMEngine::set_context(const LanguageContext& context) {
    context_ = context;
}

std::vector<MemoryRecord> LLMEngine::recall_memories(const std::string& query) {
    return memory_->retrieve_memories(query);
}

void LLMEngine::store_interaction(const std::string& input, const std::string& output) {
    memory_->store_memory(input + " | " + output, confidence_);
}

std::vector<float> LLMEngine::tokenize(const std::string& text) {
    std::vector<float> tokens;
    for (char c : text) {
        tokens.push_back(static_cast<float>(c) / 256.0f);
    }
    return tokens;
}

std::string LLMEngine::detokenize(const std::vector<float>& tokens) {
    std::string text;
    for (float token : tokens) {
        text += char(static_cast<int>(token * 256.0f));
    }
    return text;
}

Activation LLMEngine::encode_input(const std::string& input) {
    Activation activation(config_.embedding_dim, 0.0f);
    auto tokens = tokenize(input);
    
    for (size_t i = 0; i < std::min(tokens.size(), activation.size()); ++i) {
        activation[i] = tokens[i];
    }
    
    return activation;
}

std::string LLMEngine::decode_output(const Activation& output) {
    std::vector<float> tokens(output.begin(), output.end());
    return detokenize(tokens);
}

} // namespace BrainLLM
