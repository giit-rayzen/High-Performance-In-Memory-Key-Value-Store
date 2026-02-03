#include "StorageEngine.h"

bool StorageEngine::set(const std::string& key, const std::string& value){
    store_[key]= value;
    return true;
}

std::optional<std::string> StorageEngine::get(const std::string& key) const{
    auto it=store_.find(key);
    if(it==store_.end())return std::nullopt;
    return it->second;
}

bool StorageEngine::remove(const std::string& key){
    return store_.erase(key)>0;
}

bool StorageEngine::exists(const std::string& key) const {
    return store_.find(key) != store_.end();
}

size_t StorageEngine::size() const {
    return store_.size();
}

void StorageEngine::clear() {
    store_.clear();
}