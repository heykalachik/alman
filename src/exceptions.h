#pragma once
#include <stdexcept>
#include <string>

class AlmanException : public std::runtime_error {
public:
    explicit AlmanException(const std::string& msg) : std::runtime_error(msg) {}
};

class AliasStoreException : public AlmanException {
    std::string path_;
public:
    AliasStoreException(const std::string& path, const std::string& reason)
        : AlmanException("Store error [" + path + "]: " + reason), path_(path) {}
    const std::string& path() const { return path_; }
};

class DuplicateAliasException : public AlmanException {
public:
    DuplicateAliasException(const std::string& alias_name, const std::string& group)
        : AlmanException("Alias '" + alias_name + "' already exists in group '" + group + "'") {}
};

class NotFoundException : public AlmanException {
public:
    explicit NotFoundException(const std::string& what)
        : AlmanException("Not found: " + what) {}
};

class InvalidAliasException : public AlmanException {
public:
    InvalidAliasException(const std::string& name, const std::string& reason)
        : AlmanException("Invalid alias '" + name + "': " + reason) {}
};
