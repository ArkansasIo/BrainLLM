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
    
    // Utilities
    void set_attention_mask(const std::vector<bool>& mask);
    std::vector<float> get_attention_weights() const;
    
private:
    int num_heads_;
    int embedding_dim_;
    std::vector<AttentionHead> heads_;
    std::vector<bool> attention_mask_;
    
    float compute_attention_score(
        const std::vector<float>& query,
        const std::vector<float>& key
    );
    void apply_softmax(std::vector<float>& scores);
};

} // namespace BrainLLM
