#pragma once
#include "AliasGroup.h"
#include <vector>
#include <string>

class AliasStore {
    std::vector<AliasGroup> groups_;
    std::string store_path_;

public:
    explicit AliasStore(std::string path);

    void add_group(AliasGroup group);
    void remove_group(const std::string& name);
    AliasGroup* find_group(const std::string& name);
    const AliasGroup* find_group(const std::string& name) const;

    std::vector<Alias> search_all(const std::string& query) const;
    void sort_groups();
    size_t total_alias_count() const;

    void load();
    void save() const;

    const std::vector<AliasGroup>& groups() const { return groups_; }
    const std::string& store_path() const { return store_path_; }
};
