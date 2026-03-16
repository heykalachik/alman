#include "AliasStore.h"
#include "exceptions.h"
#include <algorithm>
#include <numeric>
#include <fstream>
#include <sstream>
#include <filesystem>

AliasStore::AliasStore(std::string path) : store_path_(std::move(path)) {}

void AliasStore::add_group(AliasGroup group) {
    if (std::any_of(groups_.begin(), groups_.end(),
            [&](const AliasGroup& g) { return g.name() == group.name(); }))
        throw DuplicateAliasException(group.name(), "store");
    groups_.push_back(std::move(group));
}

void AliasStore::remove_group(const std::string& name) {
    auto it = std::find_if(groups_.begin(), groups_.end(),
        [&](const AliasGroup& g) { return g.name() == name; });
    if (it == groups_.end())
        throw NotFoundException("group '" + name + "'");
    groups_.erase(it);
}

AliasGroup* AliasStore::find_group(const std::string& name) {
    auto it = std::find_if(groups_.begin(), groups_.end(),
        [&](const AliasGroup& g) { return g.name() == name; });
    return it == groups_.end() ? nullptr : &*it;
}

const AliasGroup* AliasStore::find_group(const std::string& name) const {
    auto it = std::find_if(groups_.begin(), groups_.end(),
        [&](const AliasGroup& g) { return g.name() == name; });
    return it == groups_.end() ? nullptr : &*it;
}

std::vector<Alias> AliasStore::search_all(const std::string& query) const {
    std::vector<Alias> result;
    for (const auto& group : groups_) {
        auto matches = group.search(query);
        result.insert(result.end(), matches.begin(), matches.end());
    }
    return result;
}

void AliasStore::sort_groups() {
    std::sort(groups_.begin(), groups_.end(),
        [](const AliasGroup& a, const AliasGroup& b) { return a.name() < b.name(); });
}

size_t AliasStore::total_alias_count() const {
    return std::accumulate(groups_.begin(), groups_.end(), size_t{0},
        [](size_t sum, const AliasGroup& g) { return sum + g.size(); });
}

void AliasStore::load() {
    std::ifstream file(store_path_);
    if (!file.is_open()) {
        // If file doesn't exist yet, start empty
        return;
    }

    groups_.clear();
    std::string line;
    std::string current_group_name;
    std::string current_group_desc;
    std::vector<std::pair<std::string, std::string>> current_aliases;

    auto flush_group = [&]() {
        if (!current_group_name.empty()) {
            AliasGroup group(current_group_name, current_group_desc);
            for (auto& [n, c] : current_aliases) {
                try { group.add(Alias(n, c)); }
                catch (const AlmanException&) {}
            }
            groups_.push_back(std::move(group));
            current_group_name.clear();
            current_group_desc.clear();
            current_aliases.clear();
        }
    };

    while (std::getline(file, line)) {
        if (line.empty() || line[0] == '#') continue;

        if (line.rfind("[group:", 0) == 0) {
            flush_group();
            size_t end = line.find(']');
            if (end == std::string::npos)
                throw AliasStoreException(store_path_, "malformed group header: " + line);
            current_group_name = line.substr(7, end - 7);
        } else if (line.rfind("description=", 0) == 0) {
            current_group_desc = line.substr(12);
        } else {
            size_t eq = line.find('=');
            if (eq == std::string::npos || eq == 0) continue;
            std::string name = line.substr(0, eq);
            std::string cmd = line.substr(eq + 1);
            if (!name.empty() && !cmd.empty())
                current_aliases.emplace_back(name, cmd);
        }
    }
    flush_group();
}

void AliasStore::save() const {
    // Ensure directory exists
    std::filesystem::path p(store_path_);
    if (p.has_parent_path()) {
        std::error_code ec;
        std::filesystem::create_directories(p.parent_path(), ec);
        if (ec) throw AliasStoreException(store_path_, "cannot create directory: " + ec.message());
    }

    std::ofstream file(store_path_, std::ios::trunc);
    if (!file.is_open())
        throw AliasStoreException(store_path_, "cannot open for writing");

    file << "# alman alias store v1\n";
    for (const auto& group : groups_) {
        file << "\n[group:" << group.name() << "]\n";
        file << "description=" << group.description() << "\n";
        for (const auto& alias : group.aliases()) {
            file << alias.name() << "=" << alias.command() << "\n";
        }
    }
}
