#ifndef STORAGEENGINE_H
#define STORAGEENGINE_H

/*
StorageEngine - Core data storage with multiple types

original design + enhancements:
- Multi-type support (String, List, Set, Hash)
- TTL (Time-To-Live) with automatic expiration
- Type-safe operations
- Lazy expiration (check on access, periodic cleanup)

Design Pattern: Repository Pattern
- Encapsulates data access logic
- Single source of truth
- Easy to swap storage backend (memory → disk → distributed)
*/

#include "ValueTypes.h"
#include <unordered_map>
#include <string>
#include <optional>
#include <vector>

class StorageEngine {
private:
    // Main storage: key → RedisValue (with type and expiry)
    std::unordered_map<std::string, RedisValue> store_;
    
    // Helper: Check and remove expired keys
    bool isExpired(const std::string& key);
    
    // Helper: Ensure key exists and has correct type
    bool validateType(const std::string& key, ValueType expected) const;
    
public:
    // ========== STRING OPERATIONS ==========
    
    // SET key value [EX seconds]
    bool set(const std::string& key, const std::string& value, int ttl = 0);
    
    // GET key
    std::optional<std::string> get(const std::string& key);
    
    // ========== LIST OPERATIONS ==========
    
    // LPUSH key value1 [value2 ...] - push to left (head)
    size_t lpush(const std::string& key, const std::vector<std::string>& values);
    
    // RPUSH key value1 [value2 ...] - push to right (tail)
    size_t rpush(const std::string& key, const std::vector<std::string>& values);
    
    // LPOP key - pop from left
    std::optional<std::string> lpop(const std::string& key);
    
    // RPOP key - pop from right
    std::optional<std::string> rpop(const std::string& key);
    
    // LRANGE key start stop - get range of elements
    std::vector<std::string> lrange(const std::string& key, int start, int stop);
    
    // LLEN key - get list length
    size_t llen(const std::string& key);
    
    // ========== SET OPERATIONS ==========
    
    // SADD key member1 [member2 ...] - add members to set
    size_t sadd(const std::string& key, const std::vector<std::string>& members);
    
    // SREM key member1 [member2 ...] - remove members from set
    size_t srem(const std::string& key, const std::vector<std::string>& members);
    
    // SISMEMBER key member - check if member exists
    bool sismember(const std::string& key, const std::string& member);
    
    // SMEMBERS key - get all members
    std::vector<std::string> smembers(const std::string& key);
    
    // SCARD key - get set size
    size_t scard(const std::string& key);
    
    // ========== HASH OPERATIONS ==========
    
    // HSET key field value
    bool hset(const std::string& key, const std::string& field, const std::string& value);
    
    // HGET key field
    std::optional<std::string> hget(const std::string& key, const std::string& field);
    
    // HDEL key field1 [field2 ...]
    size_t hdel(const std::string& key, const std::vector<std::string>& fields);
    
    // HEXISTS key field
    bool hexists(const std::string& key, const std::string& field);
    
    // HGETALL key - get all field-value pairs
    std::unordered_map<std::string, std::string> hgetall(const std::string& key);
    
    // HLEN key - get number of fields
    size_t hlen(const std::string& key);
    
    // ========== GENERAL OPERATIONS ==========
    
    // DEL key - delete key (any type)
    bool remove(const std::string& key);
    
    // EXISTS key
    bool exists(const std::string& key);
    
    // TYPE key - get value type
    std::optional<ValueType> type(const std::string& key);
    
    // EXPIRE key seconds - set TTL on existing key
    bool expire(const std::string& key, int seconds);
    
    // TTL key - get remaining time to live (-1 if no expiry, -2 if not exists)
    int ttl(const std::string& key);
    
    // KEYS - get all keys (for testing, not production)
    std::vector<std::string> keys() const;
    
    // DBSIZE - get number of keys
    size_t size() const;
    
    // FLUSHDB - delete all keys
    void clear();
    
    // Clean up expired keys (call periodically)
    size_t cleanupExpired();
    
    // Get raw data for persistence
    const std::unordered_map<std::string, RedisValue>& getRawData() const {
        return store_;
    }
    
    // Load raw data from persistence
    void loadRawData(const std::unordered_map<std::string, RedisValue>& data) {
        store_ = data;
    }
};

#endif // STORAGEENGINE_H
