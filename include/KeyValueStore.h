#ifndef KEYVALUESTORE_H
#define KEYVALUESTORE_H

/*
KeyValueStore - User-Facing API

Delegates work to StorageEngine
Clean separation of concerns:
- KeyValueStore: Public API, command routing
- StorageEngine: Internal storage implementation
- ThreadSafeStore: Concurrency wrapper (separate layer)

This follows the Facade Pattern:
- Simple interface hiding complex subsystem
- Easy to use, hard to misuse
*/

#include "StorageEngine.h"
#include "ValueTypes.h"
#include <string>
#include <optional>
#include <vector>
#include <unordered_map>

class KeyValueStore {
private:
    StorageEngine storage_;  //composition over inheritance
    
public:
    // ========== STRING COMMANDS ==========
    
    // SET key value [EX seconds]
    bool set(const std::string& key, const std::string& value, int ttl = 0);
    
    // GET key
    std::optional<std::string> get(const std::string& key) const;
    
    // ========== LIST COMMANDS ==========
    
    // LPUSH key value [value ...]
    size_t lpush(const std::string& key, const std::vector<std::string>& values);
    
    // RPUSH key value [value ...]
    size_t rpush(const std::string& key, const std::vector<std::string>& values);
    
    // LPOP key
    std::optional<std::string> lpop(const std::string& key);
    
    // RPOP key
    std::optional<std::string> rpop(const std::string& key);
    
    // LRANGE key start stop
    std::vector<std::string> lrange(const std::string& key, int start, int stop) const;
    
    // LLEN key
    size_t llen(const std::string& key) const;
    
    // ========== SET COMMANDS ==========
    
    // SADD key member [member ...]
    size_t sadd(const std::string& key, const std::vector<std::string>& members);
    
    // SREM key member [member ...]
    size_t srem(const std::string& key, const std::vector<std::string>& members);
    
    // SISMEMBER key member
    bool sismember(const std::string& key, const std::string& member) const;
    
    // SMEMBERS key
    std::vector<std::string> smembers(const std::string& key) const;
    
    // SCARD key
    size_t scard(const std::string& key) const;
    
    // ========== HASH COMMANDS ==========
    
    // HSET key field value
    bool hset(const std::string& key, const std::string& field, const std::string& value);
    
    // HGET key field
    std::optional<std::string> hget(const std::string& key, const std::string& field) const;
    
    // HDEL key field [field ...]
    size_t hdel(const std::string& key, const std::vector<std::string>& fields);
    
    // HEXISTS key field
    bool hexists(const std::string& key, const std::string& field) const;
    
    // HGETALL key
    std::unordered_map<std::string, std::string> hgetall(const std::string& key) const;
    
    // HLEN key
    size_t hlen(const std::string& key) const;
    
    // ========== GENERAL COMMANDS ==========
    
    // DEL key (renamed from remove for Redis compatibility)
    bool del(const std::string& key);
    
    // EXISTS key
    bool exists(const std::string& key) const;
    
    // TYPE key
    std::optional<ValueType> type(const std::string& key) const;
    
    // EXPIRE key seconds
    bool expire(const std::string& key, int seconds);
    
    // TTL key
    int ttl(const std::string& key) const;
    
    // KEYS - get all keys
    std::vector<std::string> keys() const;
    
    // DBSIZE
    size_t size() const;
    
    // FLUSHDB
    void clear();
    
    // Internal: access storage for persistence/thread-safety layers
    StorageEngine& getStorage() { return storage_; }
    const StorageEngine& getStorage() const { return storage_; }
};

#endif // KEYVALUESTORE_H