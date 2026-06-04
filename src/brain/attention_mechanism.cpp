#include "attention_mechanism.h"
#include <cmath>
#include <numeric>
#include <algorithm>
#include <limits>
#include <stdexcept>

namespace BrainLLM {

namespace {

constexpr float kMaskedScore = -1.0e9f;
constexpr float kEpsilon = 1.0e-8f;

int safe_token_count(size_t input_size, int embedding_dim, int requested_context_length) {
    if (embedding_dim <= 0 || input_size == 0) {
        return 0;
    }

    const int available_tokens = static_cast<int>(input_size / static_cast<size_t>(embedding_dim));
    if (available_tokens > 0) {
        return requested_context_length > 0
            ? std::min(available_tokens, requested_context_length)
            : available_tokens;
    }

    return 1;
}

float vector_norm(const std::vector<float>& values) {
    float sum = 0.0f;
    for (float value : values) {
        sum += value * value;
    }
    return std::sqrt(sum);
}

} // namespace

AttentionMechanism::AttentionMechanism(int num_heads, int embedding_dim)
    : num_heads_(std::max(1, num_heads)),
      embedding_dim_(std::max(1, embedding_dim)),
      causal_mask_enabled_(false) {
    heads_.resize(num_heads_);
    attention_mask_.resize(embedding_dim_, true);
    last_attention_weights_.resize(1, 1.0f);
}

std::vector<float> AttentionMechanism::forward(
    const std::vector<float>& query,
    const std::vector<float>& key,
    const std::vector<float>& value) {

    if (query.empty() || key.empty() || value.empty()) {
        last_attention_weights_.clear();
        return {};
    }

    const int token_dim = static_cast<int>(std::min(key.size(), value.size()));
    return scaled_dot_product_attention(query, key, value, 1, token_dim, 0);
}

std::vector<float> AttentionMechanism::multi_head_attention(
    const std::vector<float>& input,
    int context_length) {

    if (input.empty()) {
        last_attention_weights_.clear();
        return {};
    }

    const int head_dim = std::max(1, embedding_dim_ / num_heads_);
    const int token_count = safe_token_count(input.size(), embedding_dim_, context_length);
    const int active_dim = token_count > 1
        ? std::min(embedding_dim_, static_cast<int>(input.size()) / token_count)
        : static_cast<int>(input.size());
    const int usable_dim = std::min(active_dim, head_dim * num_heads_);

    std::vector<float> output(input.size(), 0.0f);
    std::vector<float> combined_attention(token_count, 0.0f);

    for (int h = 0; h < num_heads_; ++h) {
        const int head_start = h * head_dim;
        if (head_start >= usable_dim) {
            break;
        }

        const int current_head_dim = std::min(head_dim, usable_dim - head_start);
        std::vector<float> head_tokens;
        head_tokens.reserve(static_cast<size_t>(token_count * current_head_dim));

        for (int token = 0; token < token_count; ++token) {
            const int token_offset = token * active_dim;
            for (int dim = 0; dim < current_head_dim; ++dim) {
                const size_t index = static_cast<size_t>(token_offset + head_start + dim);
                head_tokens.push_back(index < input.size() ? input[index] : 0.0f);
            }
        }

        const int query_index = std::max(0, token_count - 1);
        const size_t query_offset = static_cast<size_t>(query_index * current_head_dim);
        std::vector<float> query(
            head_tokens.begin() + query_offset,
            head_tokens.begin() + query_offset + current_head_dim);

        std::vector<float> head_output = scaled_dot_product_attention(
            query,
            head_tokens,
            head_tokens,
            token_count,
            current_head_dim,
            query_index);

        for (int dim = 0; dim < current_head_dim; ++dim) {
            const size_t output_index = static_cast<size_t>(query_index * active_dim + head_start + dim);
            if (output_index < output.size() && static_cast<size_t>(dim) < head_output.size()) {
                output[output_index] = head_output[dim];
            }
        }

        for (int token = 0; token < token_count && static_cast<size_t>(token) < last_attention_weights_.size(); ++token) {
            combined_attention[token] += last_attention_weights_[token];
        }
    }

    const float normalizer = static_cast<float>(std::max(1, num_heads_));
    for (float& weight : combined_attention) {
        weight /= normalizer;
    }
    last_attention_weights_ = combined_attention;

    return output;
}

std::vector<float> AttentionMechanism::self_attention(
    const std::vector<float>& input,
    int context_length) {
    return multi_head_attention(input, context_length);
}

void AttentionMechanism::set_attention_mask(const std::vector<bool>& mask) {
    attention_mask_ = mask;
}

void AttentionMechanism::clear_attention_mask() {
    attention_mask_.assign(static_cast<size_t>(embedding_dim_), true);
}

void AttentionMechanism::enable_causal_mask(bool enabled) {
    causal_mask_enabled_ = enabled;
}

std::vector<float> AttentionMechanism::get_attention_weights() const {
    return last_attention_weights_;
}

float AttentionMechanism::get_attention_entropy() const {
    float entropy = 0.0f;
    for (float weight : last_attention_weights_) {
        if (weight > kEpsilon) {
            entropy -= weight * std::log(weight);
        }
    }
    return entropy;
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

std::vector<float> AttentionMechanism::scaled_dot_product_attention(
    const std::vector<float>& query,
    const std::vector<float>& keys,
    const std::vector<float>& values,
    int token_count,
    int token_dim,
    int query_index) {

    if (query.empty() || keys.empty() || values.empty() || token_count <= 0 || token_dim <= 0) {
        last_attention_weights_.clear();
        return {};
    }

    const int available_key_tokens = static_cast<int>(keys.size() / static_cast<size_t>(token_dim));
    const int available_value_tokens = static_cast<int>(values.size() / static_cast<size_t>(token_dim));
    const int active_tokens = std::max(0, std::min({token_count, available_key_tokens, available_value_tokens}));
    if (active_tokens == 0) {
        last_attention_weights_.clear();
        return {};
    }

    std::vector<float> scores(static_cast<size_t>(active_tokens), kMaskedScore);
    for (int token = 0; token < active_tokens; ++token) {
        const bool masked_by_user = static_cast<size_t>(token) < attention_mask_.size() && !attention_mask_[token];
        const bool masked_by_causal = causal_mask_enabled_ && token > query_index;
        if (masked_by_user || masked_by_causal) {
            continue;
        }

        float dot_product = 0.0f;
        const int comparable_dim = std::min(token_dim, static_cast<int>(query.size()));
        const int key_offset = token * token_dim;
        for (int dim = 0; dim < comparable_dim; ++dim) {
            dot_product += query[dim] * keys[static_cast<size_t>(key_offset + dim)];
        }

        scores[static_cast<size_t>(token)] = dot_product / std::sqrt(static_cast<float>(std::max(1, comparable_dim)));
    }

    const bool all_masked = std::all_of(scores.begin(), scores.end(), [](float score) {
        return score <= kMaskedScore * 0.5f;
    });

    if (all_masked) {
        std::fill(scores.begin(), scores.end(), 1.0f / static_cast<float>(scores.size()));
    } else {
        apply_softmax(scores);
    }

    last_attention_weights_ = scores;

    std::vector<float> output(static_cast<size_t>(token_dim), 0.0f);
    for (int token = 0; token < active_tokens; ++token) {
        const float weight = scores[static_cast<size_t>(token)];
        const int value_offset = token * token_dim;
        for (int dim = 0; dim < token_dim; ++dim) {
            output[static_cast<size_t>(dim)] += weight * values[static_cast<size_t>(value_offset + dim)];
        }
    }

    const float norm = vector_norm(output);
    if (norm > 1.0f) {
        for (float& value : output) {
            value /= norm;
        }
    }

    return output;
}

void AttentionMechanism::apply_softmax(std::vector<float>& scores) {
    if (scores.empty()) {
        return;
    }

    float max_score = *std::max_element(scores.begin(), scores.end());
    if (!std::isfinite(max_score)) {
        const float uniform = 1.0f / static_cast<float>(scores.size());
        std::fill(scores.begin(), scores.end(), uniform);
        return;
    }

    float sum = 0.0f;
    
    for (auto& score : scores) {
        if (score <= kMaskedScore * 0.5f) {
            score = 0.0f;
        } else {
            score = std::exp(score - max_score);
        }
        sum += score;
    }

    if (sum <= kEpsilon || !std::isfinite(sum)) {
        const float uniform = 1.0f / static_cast<float>(scores.size());
        std::fill(scores.begin(), scores.end(), uniform);
        return;
    }

    for (auto& score : scores) {
        score /= sum;
    }
}

} // namespace BrainLLM
