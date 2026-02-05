#ifndef VALUETYPES_H
#define VALUETYPES_H

/*
ValueTypes.h - Redis Data Type Definitions

Supports 4 core Redis data types:
1. STRING - Simple key-value (most common)
2. LIST   - Ordered collection (double-ended queue)
3. SET    - Unordered unique elements
4. HASH   - Field-value pairs (like nested map)

Why use variant?
- Type-safe union (vs void* or inheritance)
- No dynamic allocation overhead
- std::visit for type-safe operations
*/

#include <string>
#include <vector>
#include <unordered_set>
#include <unordered_map>
#include <variant>
#include <chrono>
#include <optional>

// Forward declarations
class RedisValue;

// Redis data type enums
enum class ValueType {
    STRING,
    LIST,
    SET,
    HASH
};

// Type aliases for clarity
using RedisString = std::string;
using RedisList = std::vector<std::string>;
using RedisSet = std::unordered_set<std::string>;
using RedisHash = std::unordered_map<std::string, std::string>;

// std::variant - type-safe union (C++17)
// Can hold ONE of these types at a time
using RedisData = std::variant<RedisString, RedisList, RedisSet, RedisHash>;

// Time point for TTL (Time-To-Live)
using TimePoint = std::chrono::system_clock::time_point;

// Complete Redis value with metadata
struct RedisValue {
    RedisData data;                          // Actual data (string/list/set/hash)
    std::optional<TimePoint> expiry;         // Optional expiration time
    
    // Default constructor (required for map operations)
    RedisValue() : data(RedisString("")), expiry(std::nullopt) {}
    
    // Constructor for non-expiring values
    explicit RedisValue(RedisData d) 
        : data(std::move(d)), expiry(std::nullopt) {}
    
    // Constructor with TTL (Time-To-Live in seconds)
    RedisValue(RedisData d, int ttl_seconds) 
        : data(std::move(d)) {
        if (ttl_seconds > 0) {
            expiry = std::chrono::system_clock::now() + 
                     std::chrono::seconds(ttl_seconds);
        }
    }
    
    // Check if value has expired
    bool isExpired() const {
        if (!expiry.has_value()) return false;
        return std::chrono::system_clock::now() > expiry.value();
    }
    
    // Get type of stored data
    ValueType getType() const {
        // std::visit - type-safe way to check variant type
        return std::visit([](auto&& arg) -> ValueType {
            using T = std::decay_t<decltype(arg)>;
            if constexpr (std::is_same_v<T, RedisString>) return ValueType::STRING;
            else if constexpr (std::is_same_v<T, RedisList>) return ValueType::LIST;
            else if constexpr (std::is_same_v<T, RedisSet>) return ValueType::SET;
            else if constexpr (std::is_same_v<T, RedisHash>) return ValueType::HASH;
        }, data);
    }
};

// Helper functions for type checking and conversion
inline std::string typeToString(ValueType type) {
    switch(type) {
        case ValueType::STRING: return "string";
        case ValueType::LIST:   return "list";
        case ValueType::SET:    return "set";
        case ValueType::HASH:   return "hash";
        default: return "unknown";
    }
}

// ============================================================================
// KEY CONCEPTS EXPLAINED:
// ============================================================================
//
// 1. std::variant (C++17):
//    - Type-safe union - holds ONE type at a time
//    - Better than: void*, inheritance, or separate variables
//    - Access with: std::get<T>, std::visit, std::holds_alternative
//
//    Example:
//      std::variant<int, string> v = 42;
//      std::get<int>(v);        // OK, returns 42
//      std::get<string>(v);     // throws std::bad_variant_access
//
// 2. std::optional (C++17):
//    - Represents "value or nothing"
//    - Better than: nullptr, magic values (-1, ""), exceptions
//    
//    Example:
//      std::optional<int> result = findUser(id);
//      if (result.has_value()) {
//          cout << result.value();
//      }
//
// 3. constexpr if (C++17):
//    - Compile-time conditional
//    - Code eliminated if false (zero overhead)
//    
//    Example:
//      if constexpr (std::is_same_v<T, int>) {
//          // Only compiled if T is int
//      }
//
// 4. std::visit:
//    - Apply function to variant's current type
//    - Type-safe, no casting needed
//    - Compiler ensures all cases handled
//
// 5. Time-To-Live (TTL):
//    - Keys auto-expire after duration
//    - Redis uses this for caching
//    - std::chrono for time arithmetic
//
// ============================================================================

#endif // VALUETYPES_H