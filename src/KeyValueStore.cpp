#include "KeyValueStore.h"

bool KeyValueStore::set(const std::string& key, const std::string& value) {
    return storage_.set(key, value);
}

std::optional<std::string> KeyValueStore::get(const std::string& key) const {
    return storage_.get(key);
}

bool KeyValueStore::remove(const std::string& key) {
    return storage_.remove(key);
}

bool KeyValueStore::exists(const std::string& key) const {
    return storage_.exists(key);
}

size_t KeyValueStore::size() const {
    return storage_.size();
}

void KeyValueStore::clear() {
    storage_.clear();
}
