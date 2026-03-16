#include "ZshExporter.h"

std::string ZshExporter::format_alias(const Alias& a) const {
    return "alias " + a.name() + "='" + a.command() + "'";
}

std::string ZshExporter::header() const {
    return "# Managed by alman — zsh aliases";
}
