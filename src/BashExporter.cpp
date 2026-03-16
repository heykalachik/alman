#include "BashExporter.h"

std::string BashExporter::format_alias(const Alias& a) const {
    return "alias " + a.name() + "=\"" + a.command() + "\"";
}

std::string BashExporter::header() const {
    return "# Managed by alman — bash aliases";
}
