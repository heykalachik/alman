#include "Displayable.h"
#include "Alias.h"
#include "AliasGroup.h"
#include "AliasStore.h"
#include "ShellExporter.h"
#include "ZshExporter.h"
#include "BashExporter.h"
#include "exceptions.h"
#include "tui/App.h"
#include <iostream>
#include <vector>
#include <algorithm>
#include <memory>
#include <time.h>

// ── Polymorphism demo (printed before TUI starts) ────────────────────────────

static void demo_polymorphism() {
    std::cout << "=== alman demo ===\n\n";

    // 1. Create aliases with operator overloads
    Alias a1("gs",  "git status",  "Show repo status");
    Alias a2("gp",  "git push",    "Push to remote");
    Alias a3("gl",  "git log --oneline -10");

    // operator<< demo
    std::cout << "Alias via operator<<: " << a1 << "\n";

    // operator== demo
    std::cout << "a1 == a1: " << std::boolalpha << (a1 == a1) << "\n";
    std::cout << "a1 == a2: " << std::boolalpha << (a1 == a2) << "\n\n";

    // 2. Build group with operator+=
    AliasGroup git_group("git", "Git shortcuts");
    git_group += a1;
    git_group += a2;
    git_group += a3;

    // sort_by_name uses std::sort + operator<
    git_group.sort_by_name();

    // contains uses std::any_of
    std::cout << "Group contains 'gs': " << git_group.contains("gs") << "\n";

    // search uses std::copy_if
    auto found = git_group.search("git");
    std::cout << "Search 'git' => " << found.size() << " results\n\n";

    // 3. Polymorphic vector of Displayable* — virtual dispatch demo
    std::vector<Displayable*> displayables;
    displayables.push_back(&a1);
    displayables.push_back(&a2);
    displayables.push_back(&git_group);

    std::cout << "=== Polymorphic display() via Displayable* ===\n";
    std::for_each(displayables.begin(), displayables.end(),
        [](Displayable* d) { d->print(); });

    std::cout << "\n=== Summaries ===\n";
    std::for_each(displayables.begin(), displayables.end(),
        [](Displayable* d) { std::cout << d->summary() << "\n"; });

    // 4. ShellExporter polymorphism via unique_ptr<ShellExporter>
    std::cout << "\n=== Shell export formats ===\n";
    std::vector<std::unique_ptr<ShellExporter>> exporters;
    exporters.push_back(std::make_unique<ZshExporter>("/tmp/zshrc_preview"));
    exporters.push_back(std::make_unique<BashExporter>("/tmp/bashrc_preview"));

    for (const auto& exp : exporters) {
        std::cout << exp->shell_name() << ": " << exp->format_alias(a1) << "\n";
    }

    // 5. Exception handling
    std::cout << "\n=== Exception handling ===\n";
    try {
        Alias bad("", "something");
    } catch (const InvalidAliasException& e) {
        std::cout << "Caught InvalidAliasException: " << e.what() << "\n";
    }
    try {
        git_group += a1; // duplicate
    } catch (const DuplicateAliasException& e) {
        std::cout << "Caught DuplicateAliasException: " << e.what() << "\n";
    }

    std::cout << "\nStarting TUI in 1 second...\n";
}

// ── Entry point ───────────────────────────────────────────────────────────────

int main() {
    try {
        demo_polymorphism();

        // Small pause so user can read demo output
        struct timespec ts{1, 0};
        nanosleep(&ts, nullptr);

        App app("");
        app.run();
        std::cout << "Goodbye!\n";
        return 0;
    } catch (const AlmanException& e) {
        std::cerr << "Fatal error: " << e.what() << "\n";
        return 1;
    } catch (const std::exception& e) {
        std::cerr << "Unexpected error: " << e.what() << "\n";
        return 1;
    }
}
