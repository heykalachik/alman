#include "Alias.h"
#include "exceptions.h"
#include <chrono>
#include <sstream>
#include <iomanip>
#include <regex>

static std::string current_date() {
    auto now = std::chrono::system_clock::now();
    std::time_t t = std::chrono::system_clock::to_time_t(now);
    std::ostringstream oss;
    oss << std::put_time(std::localtime(&t), "%Y-%m-%d");
    return oss.str();
}

Alias::Alias(std::string name, std::string command, std::string description)
    : name_(std::move(name)), command_(std::move(command)),
      description_(std::move(description)), created_at_(current_date())
{
    if (name_.empty())
        throw InvalidAliasException(name_, "name cannot be empty");
    for (char c : name_)
        if (c == ' ' || c == '\t')
            throw InvalidAliasException(name_, "name cannot contain whitespace");
    if (command_.empty())
        throw InvalidAliasException(name_, "command cannot be empty");
}

std::string Alias::display() const {
    return "alias " + name_ + "='" + std::regex_replace(command_, std::regex("'"), "'\\''") + "'";
}

std::string Alias::summary() const {
    return description_.empty() ? name_ + " — " + command_ : name_ + " — " + description_;
}

std::ostream& operator<<(std::ostream& os, const Alias& a) {
    os << "Alias{name=" << a.name_ << ", cmd=" << a.command_;
    if (!a.description_.empty()) os << ", desc=" << a.description_;
    os << "}";
    return os;
}
