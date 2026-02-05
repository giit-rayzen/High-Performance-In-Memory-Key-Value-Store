#include "../include/StorageEngine.h"
#include <algorithm>

// ========== HELPER FUNCTIONS ==========

bool StorageEngine::isExpired(const std::string& key) {
    auto it = store_.find(key);
    if (it == store_.end()) return false;
    
    if (it->second.isExpired()) {
        store_.erase(it);  // Lazy deletion: remove on access
        return true;
    }
    return false;
}

bool StorageEngine::validateType(const std::string& key, ValueType expected) const {
    auto it = store_.find(key);
    if (it == store_.end()) return false;
    return it->second.getType() == expected;
}

// ========== STRING OPERATIONS ==========

bool StorageEngine::set(const std::string& key, const std::string& value, int ttl) {
    // Create RedisValue with string data
    RedisValue redis_val(RedisString(value), ttl);
    
    // Insert or update
    store_[key] = std::move(redis_val);
    return true;
}

std::optional<std::string> StorageEngine::get(const std::string& key) {
    // Check expiration first
    if (isExpired(key)) return std::nullopt;
    
    auto it = store_.find(key);
    if (it == store_.end()) return std::nullopt;
    
    // Ensure it's a string type
    if (it->second.getType() != ValueType::STRING) {
        return std::nullopt;  // Wrong type error
    }
    
    // Extract string from variant
    // std::get<T> - throws if wrong type (we already checked)
    return std::get<RedisString>(it->second.data);
}

// ========== LIST OPERATIONS ==========

size_t StorageEngine::lpush(const std::string& key, const std::vector<std::string>& values) {
    if (isExpired(key)) store_.erase(key);
    
    auto it = store_.find(key);
    
    if (it == store_.end()) {
        // Create new list
        RedisList list(values.rbegin(), values.rend());  // Reverse to maintain order
        store_[key] = RedisValue(list);
        return list.size();
    }
    
    // Validate it's a list
    if (it->second.getType() != ValueType::LIST) return 0;
    
    // Get reference to the list inside variant
    auto& list = std::get<RedisList>(it->second.data);
    
    // Insert at beginning (left)
    list.insert(list.begin(), values.rbegin(), values.rend());
    
    return list.size();
}

size_t StorageEngine::rpush(const std::string& key, const std::vector<std::string>& values) {
    if (isExpired(key)) store_.erase(key);
    
    auto it = store_.find(key);
    
    if (it == store_.end()) {
        // Create new list
        store_[key] = RedisValue(RedisList(values));
        return values.size();
    }
    
    if (it->second.getType() != ValueType::LIST) return 0;
    
    auto& list = std::get<RedisList>(it->second.data);
    
    // Insert at end (right)
    list.insert(list.end(), values.begin(), values.end());
    
    return list.size();
}

std::optional<std::string> StorageEngine::lpop(const std::string& key) {
    if (isExpired(key)) return std::nullopt;
    
    auto it = store_.find(key);
    if (it == store_.end() || it->second.getType() != ValueType::LIST) {
        return std::nullopt;
    }
    
    auto& list = std::get<RedisList>(it->second.data);
    if (list.empty()) return std::nullopt;
    
    // Pop from left (front)
    std::string value = list.front();
    list.erase(list.begin());
    
    // Delete key if list becomes empty
    if (list.empty()) store_.erase(it);
    
    return value;
}

std::optional<std::string> StorageEngine::rpop(const std::string& key) {
    if (isExpired(key)) return std::nullopt;
    
    auto it = store_.find(key);
    if (it == store_.end() || it->second.getType() != ValueType::LIST) {
        return std::nullopt;
    }
    
    auto& list = std::get<RedisList>(it->second.data);
    if (list.empty()) return std::nullopt;
    
    // Pop from right (back)
    std::string value = list.back();
    list.pop_back();
    
    if (list.empty()) store_.erase(it);
    
    return value;
}

std::vector<std::string> StorageEngine::lrange(const std::string& key, int start, int stop) {
    if (isExpired(key)) return {};
    
    auto it = store_.find(key);
    if (it == store_.end() || it->second.getType() != ValueType::LIST) {
        return {};
    }
    
    const auto& list = std::get<RedisList>(it->second.data);
    int size = static_cast<int>(list.size());
    
    // Handle negative indices (Python-style: -1 is last element)
    if (start < 0) start = size + start;
    if (stop < 0) stop = size + stop;
    
    // Clamp to valid range
    start = std::max(0, std::min(start, size - 1));
    stop = std::max(0, std::min(stop, size - 1));
    
    if (start > stop) return {};
    
    // Extract range
    return std::vector<std::string>(list.begin() + start, list.begin() + stop + 1);
}

size_t StorageEngine::llen(const std::string& key) {
    if (isExpired(key)) return 0;
    
    auto it = store_.find(key);
    if (it == store_.end() || it->second.getType() != ValueType::LIST) return 0;
    
    return std::get<RedisList>(it->second.data).size();
}

// ========== SET OPERATIONS ==========

size_t StorageEngine::sadd(const std::string& key, const std::vector<std::string>& members) {
    if (isExpired(key)) store_.erase(key);
    
    auto it = store_.find(key);
    
    if (it == store_.end()) {
        // Create new set
        RedisSet new_set(members.begin(), members.end());
        store_[key] = RedisValue(new_set);
        return new_set.size();
    }
    
    if (it->second.getType() != ValueType::SET) return 0;
    
    auto& set = std::get<RedisSet>(it->second.data);
    size_t added = 0;
    
    for (const auto& member : members) {
        if (set.insert(member).second) {  // .second is true if inserted (not duplicate)
            added++;
        }
    }
    
    return added;
}

size_t StorageEngine::srem(const std::string& key, const std::vector<std::string>& members) {
    if (isExpired(key)) return 0;
    
    auto it = store_.find(key);
    if (it == store_.end() || it->second.getType() != ValueType::SET) return 0;
    
    auto& set = std::get<RedisSet>(it->second.data);
    size_t removed = 0;
    
    for (const auto& member : members) {
        removed += set.erase(member);  // erase returns number removed (0 or 1)
    }
    
    if (set.empty()) store_.erase(it);
    
    return removed;
}

bool StorageEngine::sismember(const std::string& key, const std::string& member) {
    if (isExpired(key)) return false;
    
    auto it = store_.find(key);
    if (it == store_.end() || it->second.getType() != ValueType::SET) return false;
    
    const auto& set = std::get<RedisSet>(it->second.data);
    return set.find(member) != set.end();
}

std::vector<std::string> StorageEngine::smembers(const std::string& key) {
    if (isExpired(key)) return {};
    
    auto it = store_.find(key);
    if (it == store_.end() || it->second.getType() != ValueType::SET) return {};
    
    const auto& set = std::get<RedisSet>(it->second.data);
    return std::vector<std::string>(set.begin(), set.end());
}

size_t StorageEngine::scard(const std::string& key) {
    if (isExpired(key)) return 0;
    
    auto it = store_.find(key);
    if (it == store_.end() || it->second.getType() != ValueType::SET) return 0;
    
    return std::get<RedisSet>(it->second.data).size();
}

// ========== HASH OPERATIONS ==========

bool StorageEngine::hset(const std::string& key, const std::string& field, const std::string& value) {
    if (isExpired(key)) store_.erase(key);
    
    auto it = store_.find(key);
    
    if (it == store_.end()) {
        // Create new hash
        RedisHash hash;
        hash[field] = value;
        store_[key] = RedisValue(hash);
        return true;
    }
    
    if (it->second.getType() != ValueType::HASH) return false;
    
    auto& hash = std::get<RedisHash>(it->second.data);
    hash[field] = value;
    return true;
}

std::optional<std::string> StorageEngine::hget(const std::string& key, const std::string& field) {
    if (isExpired(key)) return std::nullopt;
    
    auto it = store_.find(key);
    if (it == store_.end() || it->second.getType() != ValueType::HASH) return std::nullopt;
    
    const auto& hash = std::get<RedisHash>(it->second.data);
    auto field_it = hash.find(field);
    
    if (field_it == hash.end()) return std::nullopt;
    return field_it->second;
}

size_t StorageEngine::hdel(const std::string& key, const std::vector<std::string>& fields) {
    if (isExpired(key)) return 0;
    
    auto it = store_.find(key);
    if (it == store_.end() || it->second.getType() != ValueType::HASH) return 0;
    
    auto& hash = std::get<RedisHash>(it->second.data);
    size_t deleted = 0;
    
    for (const auto& field : fields) {
        deleted += hash.erase(field);
    }
    
    if (hash.empty()) store_.erase(it);
    
    return deleted;
}

bool StorageEngine::hexists(const std::string& key, const std::string& field) {
    if (isExpired(key)) return false;
    
    auto it = store_.find(key);
    if (it == store_.end() || it->second.getType() != ValueType::HASH) return false;
    
    const auto& hash = std::get<RedisHash>(it->second.data);
    return hash.find(field) != hash.end();
}

std::unordered_map<std::string, std::string> StorageEngine::hgetall(const std::string& key) {
    if (isExpired(key)) return {};
    
    auto it = store_.find(key);
    if (it == store_.end() || it->second.getType() != ValueType::HASH) return {};
    
    return std::get<RedisHash>(it->second.data);
}

size_t StorageEngine::hlen(const std::string& key) {
    if (isExpired(key)) return 0;
    
    auto it = store_.find(key);
    if (it == store_.end() || it->second.getType() != ValueType::HASH) return 0;
    
    return std::get<RedisHash>(it->second.data).size();
}

// ========== GENERAL OPERATIONS ==========

bool StorageEngine::remove(const std::string& key) {
    return store_.erase(key) > 0;
}

bool StorageEngine::exists(const std::string& key) {
    if (isExpired(key)) return false;
    return store_.find(key) != store_.end();
}

std::optional<ValueType> StorageEngine::type(const std::string& key) {
    if (isExpired(key)) return std::nullopt;
    
    auto it = store_.find(key);
    if (it == store_.end()) return std::nullopt;
    
    return it->second.getType();
}

bool StorageEngine::expire(const std::string& key, int seconds) {
    if (isExpired(key)) return false;
    
    auto it = store_.find(key);
    if (it == store_.end()) return false;
    
    if (seconds > 0) {
        it->second.expiry = std::chrono::system_clock::now() + 
                            std::chrono::seconds(seconds);
    } else {
        it->second.expiry = std::nullopt;  // Remove expiry
    }
    
    return true;
}

int StorageEngine::ttl(const std::string& key) {
    auto it = store_.find(key);
    if (it == store_.end()) return -2;  // Key doesn't exist
    
    if (!it->second.expiry.has_value()) return -1;  // No expiry
    
    auto now = std::chrono::system_clock::now();
    auto remaining = it->second.expiry.value() - now;
    
    if (remaining.count() <= 0) {
        store_.erase(it);  // Expired
        return -2;
    }
    
    return std::chrono::duration_cast<std::chrono::seconds>(remaining).count();
}

std::vector<std::string> StorageEngine::keys() const {
    std::vector<std::string> result;
    result.reserve(store_.size());
    
    for (const auto& [key, value] : store_) {
        if (!value.isExpired()) {
            result.push_back(key);
        }
    }
    
    return result;
}

size_t StorageEngine::size() const {
    // Count non-expired keys
    size_t count = 0;
    for (const auto& [key, value] : store_) {
        if (!value.isExpired()) count++;
    }
    return count;
}

void StorageEngine::clear() {
    store_.clear();
}

size_t StorageEngine::cleanupExpired() {
    size_t removed = 0;
    
    // Can't erase while iterating, so collect expired keys first
    std::vector<std::string> expired_keys;
    
    for (const auto& [key, value] : store_) {
        if (value.isExpired()) {
            expired_keys.push_back(key);
        }
    }
    
    for (const auto& key : expired_keys) {
        store_.erase(key);
        removed++;
    }
    
    return removed;
}