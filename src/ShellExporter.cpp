#include "ShellExporter.h"
#include "exceptions.h"
#include <fstream>
#include <sstream>
#include <filesystem>
#include <algorithm>

void ShellExporter::export_group(std::ofstream& file, const AliasGroup& group) const {
    file << "\n# --- " << group.name() << " ---\n";
    std::for_each(group.aliases().begin(), group.aliases().end(),
        [&](const Alias& a) { file << format_alias(a) << "\n"; });
}

void ShellExporter::export_all(const AliasStore& store) const {
    // Read existing .zshrc content
    std::string existing;
    {
        std::ifstream in(target_path_);
        if (in.is_open()) {
            std::ostringstream ss;
            ss << in.rdbuf();
            existing = ss.str();
        }
    }

    // Cut everything from the managed section header onwards
    std::string h = header();
    auto pos = existing.find(h);
    std::string base = (pos != std::string::npos) ? existing.substr(0, pos) : existing;

    // Strip trailing whitespace/newlines left before the managed block
    while (!base.empty() && (base.back() == '\n' || base.back() == '\r' || base.back() == ' '))
        base.pop_back();

    if (backup_existing()) {
        std::filesystem::path src(target_path_);
        if (std::filesystem::exists(src)) {
            std::error_code ec;
            std::filesystem::copy_file(src, target_path_ + ".alman_backup",
                std::filesystem::copy_options::overwrite_existing, ec);
        }
    }

    std::ofstream file(target_path_, std::ios::trunc);
    if (!file.is_open())
        throw AliasStoreException(target_path_, "cannot open for export");

    file << base << "\n\n" << h << "\n";
    for (const auto& group : store.groups())
        export_group(file, group);
}
