#include "memory_system.h"
#include <algorithm>
#include <chrono>

namespace BrainLLM {

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
        scored.emplace_back(relevance, entry.record);
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
    if (query.empty()) return 0.0f;
    
    // Simple substring matching for relevance
    size_t pos = memory.find(query);
    if (pos != std::string::npos) {
        return 0.9f;
    }
    
    // Partial matching
    float match_score = 0.0f;
    for (char c : query) {
        if (memory.find(c) != std::string::npos) {
            match_score += 0.1f;
        }
    }
    
    return std::min(0.8f, match_score);
}

void MemorySystem::remove_oldest() {
    if (!memory_storage_.empty()) {
        memory_storage_.pop_front();
    }
}

} // namespace BrainLLM
