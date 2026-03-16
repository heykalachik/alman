#include "AliasGroup.h"
#include "exceptions.h"
#include <algorithm>
#include <sstream>

AliasGroup::AliasGroup(std::string name, std::string description)
    : name_(std::move(name)), description_(std::move(description)) {}

void AliasGroup::add(Alias alias) {
    if (contains(alias.name()))
        throw DuplicateAliasException(alias.name(), name_);
    aliases_.push_back(std::move(alias));
}

void AliasGroup::remove(const std::string& alias_name) {
    auto it = std::find_if(aliases_.begin(), aliases_.end(),
        [&](const Alias& a) { return a.name() == alias_name; });
    if (it == aliases_.end())
        throw NotFoundException("alias '" + alias_name + "' in group '" + name_ + "'");
    aliases_.erase(it);
}

void AliasGroup::update(const std::string& alias_name, const std::string& new_command) {
    auto it = std::find_if(aliases_.begin(), aliases_.end(),
        [&](const Alias& a) { return a.name() == alias_name; });
    if (it == aliases_.end())
        throw NotFoundException("alias '" + alias_name + "' in group '" + name_ + "'");
    it->set_command(new_command);
}

std::optional<Alias> AliasGroup::find(const std::string& name) const {
    auto it = std::find_if(aliases_.begin(), aliases_.end(),
        [&](const Alias& a) { return a.name() == name; });
    if (it == aliases_.end()) return std::nullopt;
    return *it;
}

std::vector<Alias> AliasGroup::search(const std::string& query) const {
    std::vector<Alias> result;
    std::copy_if(aliases_.begin(), aliases_.end(), std::back_inserter(result),
        [&](const Alias& a) {
            return a.name().find(query) != std::string::npos ||
                   a.command().find(query) != std::string::npos ||
                   a.description().find(query) != std::string::npos;
        });
    return result;
}

void AliasGroup::sort_by_name() {
    std::sort(aliases_.begin(), aliases_.end());
}

bool AliasGroup::contains(const std::string& name) const {
    return std::any_of(aliases_.begin(), aliases_.end(),
        [&](const Alias& a) { return a.name() == name; });
}

std::string AliasGroup::display() const {
    std::ostringstream oss;
    oss << "# [" << name_ << "]";
    if (!description_.empty()) oss << " — " << description_;
    oss << "\n";
    std::for_each(aliases_.begin(), aliases_.end(),
        [&](const Alias& a) { oss << a.display() << "\n"; });
    return oss.str();
}

std::string AliasGroup::summary() const {
    return name_ + " (" + std::to_string(aliases_.size()) + " aliases)";
}

AliasGroup& AliasGroup::operator+=(Alias alias) {
    add(std::move(alias));
    return *this;
}
