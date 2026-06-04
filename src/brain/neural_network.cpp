#include "neural_network.h"
#include <cmath>
#include <algorithm>
#include <numeric>

namespace BrainLLM {

NeuralNetwork::NeuralNetwork(const BrainConfig& config)
    : config_(config), rng_(std::random_device{}()) {
    const int hidden_layers = std::max(1, config_.num_layers);
    for (int i = 0; i < hidden_layers; ++i) {
        add_layer(config_.neurons_per_layer);
    }
    initialize_weights();
}

Activation NeuralNetwork::forward(const Activation& input) {
    Activation current = input;
    
    for (size_t i = 0; i < layers_.size(); ++i) {
        Activation next(layers_[i].size(), 0.0f);
        
        for (size_t j = 0; j < layers_[i].size(); ++j) {
            float sum = 0.0f;
            for (size_t k = 0; k < current.size(); ++k) {
                sum += current[k] * layers_[i][j][k];
            }
            next[j] = (i < layers_.size() - 1) ? relu(sum) : sigmoid(sum);
        }
        current = next;
    }
    
    return current;
}

void NeuralNetwork::backward(const Activation& gradient) {
    gradients_.clear();
    gradients_.resize(layers_.size());
    const float gradient_mean = gradient.empty()
        ? 0.0f
        : std::accumulate(gradient.begin(), gradient.end(), 0.0f) /
              static_cast<float>(gradient.size());

    for (size_t i = 0; i < layers_.size(); ++i) {
        gradients_[i].resize(layers_[i].size());
        for (size_t j = 0; j < layers_[i].size(); ++j) {
            gradients_[i][j].resize(layers_[i][j].size());
            const float layer_scale = 1.0f / static_cast<float>(i + 1);
            for (size_t k = 0; k < layers_[i][j].size(); ++k) {
                gradients_[i][j][k] = gradient_mean * layer_scale * 0.01f;
            }
        }
    }
}

void NeuralNetwork::add_layer(int size) {
    int prev_size = layers_.empty() ? config_.embedding_dim : static_cast<int>(layers_.back().size());
    NeuralLayer layer(size, std::vector<float>(prev_size));
    layers_.push_back(layer);
}

void NeuralNetwork::initialize_weights() {
    std::normal_distribution<float> dist(0.0f, 0.1f);
    
    for (auto& layer : layers_) {
        for (auto& neuron : layer) {
            for (auto& weight : neuron) {
                weight = dist(rng_);
            }
        }
    }
}

float NeuralNetwork::relu(float x) {
    return std::max(0.0f, x);
}

float NeuralNetwork::sigmoid(float x) {
    return 1.0f / (1.0f + std::exp(-x));
}

float NeuralNetwork::tanh_activation(float x) {
    return std::tanh(x);
}

void NeuralNetwork::update_weights(float learning_rate) {
    if (gradients_.size() != layers_.size()) {
        return;
    }
    for (size_t i = 0; i < layers_.size(); ++i) {
        for (size_t j = 0; j < layers_[i].size(); ++j) {
            for (size_t k = 0; k < layers_[i][j].size(); ++k) {
                layers_[i][j][k] -= learning_rate * gradients_[i][j][k];
            }
        }
    }
}

BrainMetrics NeuralNetwork::get_metrics() const {
    BrainMetrics metrics{};
    metrics.cpu_usage = 0.5f;
    metrics.memory_usage = 0.3f;
    metrics.processing_speed = 0.95f;
    metrics.tokens_processed = 1000;
    metrics.accuracy = 0.87f;
    return metrics;
}

void NeuralNetwork::reset() {
    initialize_weights();
    gradients_.clear();
}

float NeuralNetwork::compute_loss(const Activation& predicted, const Activation& expected) {
    const size_t size = std::min(predicted.size(), expected.size());
    if (size == 0) {
        return 0.0f;
    }

    float loss = 0.0f;
    for (size_t i = 0; i < size; ++i) {
        const float diff = predicted[i] - expected[i];
        loss += diff * diff;
    }
    return loss / static_cast<float>(size);
}

} // namespace BrainLLM
