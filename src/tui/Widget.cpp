#include "Widget.h"
#include <string>

std::string Widget::display() const {
    return "Widget(" + std::to_string(x_) + "," + std::to_string(y_) +
           " " + std::to_string(width_) + "x" + std::to_string(height_) + ")";
}

std::string Widget::summary() const {
    return display();
}
