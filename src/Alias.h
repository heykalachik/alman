#pragma once
#include "Displayable.h"
#include <string>
#include <ostream>

class Alias : public Displayable {
    std::string name_;
    std::string command_;
    std::string description_;
    std::string created_at_;

public:
    Alias(std::string name, std::string command, std::string description = "");

    const std::string& name() const { return name_; }
    const std::string& command() const { return command_; }
    const std::string& description() const { return description_; }
    const std::string& created_at() const { return created_at_; }

    void set_command(std::string cmd) { command_ = std::move(cmd); }
    void set_description(std::string desc) { description_ = std::move(desc); }

    std::string display() const override;
    std::string summary() const override;

    bool operator==(const Alias& other) const { return name_ == other.name_; }
    bool operator<(const Alias& other) const { return name_ < other.name_; }
    friend std::ostream& operator<<(std::ostream& os, const Alias& a);
};
