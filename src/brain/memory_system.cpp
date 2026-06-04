#include "memory_system.h"
#include <algorithm>
#include <chrono>
#include <cctype>
#include <map>
#include <set>

namespace BrainLLM {

namespace {

std::vector<std::string> memory_tokens(const std::string& text) {
    std::vector<std::string> tokens;
    std::string current;
    for (unsigned char c : text) {
        if (std::isalnum(c)) {
            current += static_cast<char>(std::tolower(c));
        } else if (!current.empty()) {
            tokens.push_back(current);
            current.clear();
        }
    }
    if (!current.empty()) {
        tokens.push_back(current);
    }
    return tokens;
}

} // namespace

MemorySystem::MemorySystem(int max_size)
    : max_size_(max_size) {}

void MemorySystem::store_memory(const std::string& content, float importance) {
    if (memory_storage_.size() >= static_cast<size_t>(max_size_)) {
        remove_oldest();
    }
    
    auto now = std::chrono::system_clock::now();
    auto duration = now.time_since_epoch();
    auto timestamp = std::chrono::duration_cast<std::chrono::seconds>(duration).count();
    
    MemoryEntry entry{};
    entry.record.content = content;
    entry.record.importance = importance;
    entry.record.timestamp = timestamp;
    entry.record.category = "general";
    entry.last_accessed = timestamp;
    
    memory_storage_.push_back(entry);
}

std::vector<MemoryRecord> MemorySystem::retrieve_memories(const std::string& query, int count) {
    std::vector<MemoryRecord> results;
    
    std::vector<std::pair<float, MemoryRecord>> scored;
    for (auto& entry : memory_storage_) {
        float relevance = calculate_relevance(entry.record.content, query);
        relevance = relevance * 0.78f + entry.record.importance * 0.22f;
        if (relevance > 0.0f || query.empty()) {
            scored.emplace_back(relevance, entry.record);
        }
    }
    
    std::sort(scored.begin(), scored.end(), 
              [](const auto& a, const auto& b) { return a.first > b.first; });
    
    for (int i = 0; i < std::min(count, static_cast<int>(scored.size())); ++i) {
        results.push_back(scored[i].second);
    }
    
    return results;
}

void MemorySystem::clear_memories() {
    memory_storage_.clear();
}

int MemorySystem::get_memory_count() const {
    return memory_storage_.size();
}

float MemorySystem::get_memory_usage() const {
    return static_cast<float>(memory_storage_.size()) / max_size_;
}

void MemorySystem::consolidate_memories() {
    // Merge similar memories
    auto now = std::chrono::system_clock::now();
    auto duration = now.time_since_epoch();
    auto current_time = std::chrono::duration_cast<std::chrono::seconds>(duration).count();
    
    for (auto& entry : memory_storage_) {
        if (entry.record.importance > 0.8f) {
            entry.record.importance += 0.05f;
        }
    }
}

void MemorySystem::decay_old_memories() {
    auto now = std::chrono::system_clock::now();
    auto duration = now.time_since_epoch();
    auto current_time = std::chrono::duration_cast<std::chrono::seconds>(duration).count();
    
    for (auto& entry : memory_storage_) {
        uint64_t age = current_time - entry.record.timestamp;
        float decay_factor = 0.95f; // Decay 5% per time period
        entry.record.importance *= decay_factor;
    }
}

void MemorySystem::categorize_memory(const std::string& content, const std::string& category) {
    for (auto& entry : memory_storage_) {
        if (entry.record.content == content) {
            entry.record.category = category;
            break;
        }
    }
}

std::vector<MemoryRecord> MemorySystem::get_memories_by_category(const std::string& category) {
    std::vector<MemoryRecord> results;
    
    for (const auto& entry : memory_storage_) {
        if (entry.record.category == category) {
            results.push_back(entry.record);
        }
    }
    
    return results;
}

float MemorySystem::calculate_relevance(const std::string& memory, const std::string& query) const {
    if (query.empty()) return 0.1f;

    std::string lower_memory = memory;
    std::string lower_query = query;
    std::transform(lower_memory.begin(), lower_memory.end(), lower_memory.begin(), ::tolower);
    std::transform(lower_query.begin(), lower_query.end(), lower_query.begin(), ::tolower);

    if (lower_memory.find(lower_query) != std::string::npos) {
        return 1.0f;
    }

    const auto memory_words = memory_tokens(memory);
    const auto query_words = memory_tokens(query);
    if (memory_words.empty() || query_words.empty()) {
        return 0.0f;
    }

    std::set<std::string> memory_set(memory_words.begin(), memory_words.end());
    std::map<std::string, int> memory_frequency;
    for (const auto& word : memory_words) {
        memory_frequency[word]++;
    }

    float overlap = 0.0f;
    float weighted = 0.0f;
    for (const auto& word : query_words) {
        if (memory_set.find(word) != memory_set.end()) {
            overlap += 1.0f;
            weighted += 1.0f + std::min(2, memory_frequency[word]) * 0.2f;
        }
    }

    const float coverage = overlap / static_cast<float>(std::max<size_t>(1, query_words.size()));
    const float density = weighted / static_cast<float>(std::max<size_t>(1, memory_words.size()));
    return std::min(1.0f, coverage * 0.75f + density * 0.25f);
}

void MemorySystem::remove_oldest() {
    if (!memory_storage_.empty()) {
        memory_storage_.pop_front();
    }
}

} // namespace BrainLLM
