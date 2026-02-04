#pragma once

#include <vector>
#include <string>
#include <map>
#include <memory>
#include <cstdint>

namespace BrainLLM {

// Neural Network Types
using NeuralLayer = std::vector<std::vector<float>>;
using Activation = std::vector<float>;
using Weight = float;

struct NeuronState {
    float activation;
    float potential;
    float bias;
    std::vector<Weight> weights;
};

struct NeuralNode {
    uint32_t id;
    NeuronState state;
    std::vector<uint32_t> connections;
};

// Memory Types
struct MemoryRecord {
    uint64_t timestamp;
    std::string content;
    float importance;
    std::string category;
};

struct MemoryBlock {
    std::vector<MemoryRecord> records;
    float attention_weight;
};

// Attention Mechanism
struct AttentionHead {
    std::vector<float> query;
    std::vector<float> key;
    std::vector<float> value;
    std::vector<float> output;
};

struct AttentionLayer {
    std::vector<AttentionHead> heads;
    int num_heads;
};

// LLM Context
struct TokenEmbedding {
    uint32_t token_id;
    std::vector<float> embedding;
};

struct LanguageContext {
    std::vector<TokenEmbedding> embeddings;
    std::string current_input;
    std::string last_output;
    float confidence;
};

// Configuration
struct BrainConfig {
    // Neural Network
    int num_layers;
    int neurons_per_layer;
    float learning_rate;
    
    // Memory
    int max_memory_size;
    float memory_decay_rate;
    
    // Attention
    int num_attention_heads;
    int attention_dim;
    
    // LLM
    int vocab_size;
    int embedding_dim;
    int context_length;
    
    // Processing
    int batch_size;
    float temperature;
};

// Brain State
enum class BrainState {
    Idle,
    Processing,
    Learning,
    Sleeping,
    Error
};

struct BrainMetrics {
    float cpu_usage;
    float memory_usage;
    float processing_speed;
    uint64_t tokens_processed;
    float accuracy;
};

} // namespace BrainLLM
