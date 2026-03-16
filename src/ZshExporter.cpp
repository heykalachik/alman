#include "ZshExporter.h"

std::string ZshExporter::format_alias(const Alias& a) const {
    return a.display();
}

std::string ZshExporter::header() const {
    return "# Managed by alman — zsh aliases";
}
