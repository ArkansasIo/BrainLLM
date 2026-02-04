#include "attention_mechanism.h"
#include <cmath>
#include <numeric>
#include <algorithm>

namespace BrainLLM {

AttentionMechanism::AttentionMechanism(int num_heads, int embedding_dim)
    : num_heads_(num_heads), embedding_dim_(embedding_dim) {
    heads_.resize(num_heads);
    attention_mask_.resize(embedding_dim, true);
}

std::vector<float> AttentionMechanism::forward(
    const std::vector<float>& query,
    const std::vector<float>& key,
    const std::vector<float>& value) {
    
    float score = compute_attention_score(query, key);
    std::vector<float> output = value;
    
    for (auto& v : output) {
        v *= score;
    }
    
    return output;
}

std::vector<float> AttentionMechanism::multi_head_attention(
    const std::vector<float>& input,
    int context_length) {
    
    std::vector<float> output(input.size(), 0.0f);
    int head_dim = embedding_dim_ / num_heads_;
    
    for (int h = 0; h < num_heads_; ++h) {
        int start = h * head_dim;
        int end = std::min(start + head_dim, static_cast<int>(input.size()));
        
        std::vector<float> head_input(input.begin() + start, input.begin() + end);
        std::vector<float> head_output = forward(head_input, head_input, head_input);
        
        for (size_t i = 0; i < head_output.size() && start + i < output.size(); ++i) {
            output[start + i] = head_output[i];
        }
    }
    
    return output;
}

void AttentionMechanism::set_attention_mask(const std::vector<bool>& mask) {
    attention_mask_ = mask;
}

std::vector<float> AttentionMechanism::get_attention_weights() const {
    std::vector<float> weights;
    for (size_t i = 0; i < attention_mask_.size(); ++i) {
        weights.push_back(attention_mask_[i] ? 1.0f : 0.0f);
    }
    return weights;
}

float AttentionMechanism::compute_attention_score(
    const std::vector<float>& query,
    const std::vector<float>& key) {
    
    if (query.empty() || key.empty()) return 0.0f;
    
    float dot_product = 0.0f;
    for (size_t i = 0; i < std::min(query.size(), key.size()); ++i) {
        dot_product += query[i] * key[i];
    }
    
    float scale = 1.0f / std::sqrt(static_cast<float>(key.size()));
    return dot_product * scale;
}

void AttentionMechanism::apply_softmax(std::vector<float>& scores) {
    float max_score = *std::max_element(scores.begin(), scores.end());
    float sum = 0.0f;
    
    for (auto& score : scores) {
        score = std::exp(score - max_score);
        sum += score;
    }
    
    for (auto& score : scores) {
        score /= sum;
    }
}

} // namespace BrainLLM
