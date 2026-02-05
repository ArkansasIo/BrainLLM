#pragma once

#include "brain_types.h"
#include <vector>
#include <string>
#include <map>

namespace BrainLLM {

// ========================================
// TRANSFORMER AND ADVANCED ARCHITECTURE
// ========================================

struct TransformerBlock {
    int layer_id;
    int num_heads;
    int hidden_dim;
    std::vector<float> weights;
    std::vector<float> biases;
};

struct FeedForward {
    int input_dim;
    int hidden_dim;
    int output_dim;
    std::vector<float> w1, b1;
    std::vector<float> w2, b2;
};

class TransformerModule {
public:
    TransformerModule(int num_layers, int num_heads, int hidden_dim);
    ~TransformerModule() = default;
    
    // Forward pass
    std::vector<float> forward(const std::vector<float>& input);
    
    // Layer operations
    std::vector<float> apply_attention(const std::vector<float>& query,
                                       const std::vector<float>& key,
                                       const std::vector<float>& value);
    
    std::vector<float> apply_feed_forward(const std::vector<float>& input);
    
    // Layer normalization
    std::vector<float> layer_norm(const std::vector<float>& input);
    
    // Residual connections
    std::vector<float> apply_residual(const std::vector<float>& input,
                                      const std::vector<float>& output);
    
private:
    std::vector<TransformerBlock> layers_;
    std::vector<FeedForward> feed_forwards_;
    int num_layers_;
    int num_heads_;
    int hidden_dim_;
};

// ========================================
// EMBEDDING AND TOKENIZATION
// ========================================

class EmbeddingLayer {
public:
    EmbeddingLayer(int vocab_size, int embedding_dim);
    ~EmbeddingLayer() = default;
    
    // Embedding operations
    std::vector<float> embed_token(int token_id);
    std::vector<int> tokenize(const std::string& text);
    std::string detokenize(const std::vector<int>& tokens);
    
    // Position encoding
    std::vector<float> get_positional_encoding(int position, int dim);
    
    // Learn embeddings
    void update_embeddings(const std::vector<int>& tokens,
                          const std::vector<float>& gradients);
    
private:
    int vocab_size_;
    int embedding_dim_;
    std::map<int, std::vector<float>> embedding_matrix_;
    
    void initialize_embeddings();
};

// ========================================
// SEQUENCE TO SEQUENCE (SEQ2SEQ)
// ========================================

class Seq2SeqModel {
public:
    Seq2SeqModel(int vocab_size, int embedding_dim, int hidden_dim);
    ~Seq2SeqModel() = default;
    
    // Encoding
    std::vector<float> encode(const std::string& input);
    
    // Decoding
    std::string decode(const std::vector<float>& encoder_output);
    
    // Full sequence transformation
    std::string transform_sequence(const std::string& input);
    
    // Attention mechanism
    std::vector<float> compute_attention(const std::vector<float>& decoder_state);
    
private:
    int vocab_size_;
    int embedding_dim_;
    int hidden_dim_;
    
    class Encoder {
    public:
        Encoder(int hidden_dim);
        std::vector<float> encode(const std::vector<int>& tokens);
    private:
        int hidden_dim_;
    };
    
    class Decoder {
    public:
        Decoder(int hidden_dim);
        std::string decode(const std::vector<float>& context);
    private:
        int hidden_dim_;
    };
};

// ========================================
// RECURRENT NEURAL NETWORK (RNN/LSTM)
// ========================================

struct LSTMState {
    std::vector<float> h;  // Hidden state
    std::vector<float> c;  // Cell state
};

class LSTMCell {
public:
    LSTMCell(int input_size, int hidden_size);
    ~LSTMCell() = default;
    
    // Forward pass
    LSTMState forward(const std::vector<float>& input, const LSTMState& prev_state);
    
    // Backward pass
    void backward(const std::vector<float>& gradient);
    
    // Update weights
    void update_weights(float learning_rate);
    
private:
    int input_size_;
    int hidden_size_;
    
    // Weights for gates: forget, input, cell, output
    std::vector<float> w_forget_, b_forget_;
    std::vector<float> w_input_, b_input_;
    std::vector<float> w_cell_, b_cell_;
    std::vector<float> w_output_, b_output_;
};

class LSTMNetwork {
public:
    LSTMNetwork(int num_layers, int input_size, int hidden_size);
    ~LSTMNetwork() = default;
    
    // Process sequence
    std::vector<std::vector<float>> forward_sequence(
        const std::vector<std::vector<float>>& sequence);
    
    std::string process_text(const std::string& text);
    
private:
    std::vector<LSTMCell> cells_;
    int num_layers_;
    int hidden_size_;
};

} // namespace BrainLLM
