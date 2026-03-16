#pragma once
#include <string>
#include <iostream>

class Displayable {
public:
    virtual ~Displayable() = default;
    virtual std::string display() const = 0;
    virtual std::string summary() const = 0;
    virtual void print() const { std::cout << display() << "\n"; }
};
