#pragma once
#include "Displayable.h"
#include "Alias.h"
#include <vector>
#include <string>
#include <optional>

class AliasGroup : public Displayable {
    std::string name_;
    std::string description_;
    std::vector<Alias> aliases_;

public:
    AliasGroup(std::string name, std::string description = "");

    void add(Alias alias);
    void remove(const std::string& alias_name);
    void update(const std::string& alias_name, const std::string& new_command);
    std::optional<Alias> find(const std::string& name) const;

    std::vector<Alias> search(const std::string& query) const;
    void sort_by_name();
    bool contains(const std::string& name) const;

    const std::vector<Alias>& aliases() const { return aliases_; }
    const std::string& name() const { return name_; }
    const std::string& description() const { return description_; }
    void set_description(std::string desc) { description_ = std::move(desc); }
    size_t size() const { return aliases_.size(); }

    std::string display() const override;
    std::string summary() const override;

    AliasGroup& operator+=(Alias alias);
    bool operator==(const AliasGroup& o) const { return name_ == o.name_; }
};
