/*
User-facing API
Delegates work to StorageEngine
*/

#pragma once
#include <string>
#include <optional>
#include "StorageEngine.h"

class KeyValueStore {
private:
    StorageEngine storage_;
public:
    bool set(const std::string& key, const std::string& value);
    std::optional<std::string> get(const std::string& key) const;
    bool remove(const std::string& key);
    bool exists(const std::string& key) const;
    size_t size() const;
    void clear();
};
