#pragma once

#include "brain_types.h"

namespace BrainLLM {

class AttentionMechanism {
public:
    AttentionMechanism(int num_heads, int embedding_dim);
    ~AttentionMechanism() = default;
    
    // Attention computation
    std::vector<float> forward(
        const std::vector<float>& query,
        const std::vector<float>& key,
        const std::vector<float>& value
    );
    
    // Multi-head attention
    std::vector<float> multi_head_attention(
        const std::vector<float>& input,
        int context_length
    );

    // Transformer-style helpers
    std::vector<float> self_attention(
        const std::vector<float>& input,
        int context_length
    );
    
    // Utilities
    void set_attention_mask(const std::vector<bool>& mask);
    void clear_attention_mask();
    void enable_causal_mask(bool enabled);
    std::vector<float> get_attention_weights() const;
    float get_attention_entropy() const;
    
private:
    int num_heads_;
    int embedding_dim_;
    std::vector<AttentionHead> heads_;
    std::vector<bool> attention_mask_;
    std::vector<float> last_attention_weights_;
    bool causal_mask_enabled_;
    
    float compute_attention_score(
        const std::vector<float>& query,
        const std::vector<float>& key
    );
    std::vector<float> scaled_dot_product_attention(
        const std::vector<float>& query,
        const std::vector<float>& keys,
        const std::vector<float>& values,
        int token_count,
        int token_dim,
        int query_index
    );
    void apply_softmax(std::vector<float>& scores);
};

} // namespace BrainLLM
