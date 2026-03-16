#pragma once
#include "ShellExporter.h"

class BashExporter : public ShellExporter {
public:
    explicit BashExporter(std::string path) : ShellExporter(std::move(path)) {}
    std::string format_alias(const Alias& a) const override;
    std::string header() const override;
    std::string shell_name() const override { return "bash"; }
};
