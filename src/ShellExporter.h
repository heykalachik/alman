#pragma once
#include "Alias.h"
#include "AliasGroup.h"
#include "AliasStore.h"
#include <string>
#include <fstream>

class ShellExporter {
protected:
    std::string target_path_;

public:
    explicit ShellExporter(std::string path) : target_path_(std::move(path)) {}
    virtual ~ShellExporter() = default;

    virtual std::string format_alias(const Alias& a) const = 0;
    virtual std::string header() const = 0;
    virtual std::string shell_name() const = 0;
    virtual bool backup_existing() const { return true; }

    void export_group(std::ofstream& file, const AliasGroup& group) const;
    void export_all(const AliasStore& store) const;

    const std::string& target_path() const { return target_path_; }
};
