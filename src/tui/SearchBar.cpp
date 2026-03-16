#include "SearchBar.h"
#include "Terminal.h"
#include <string>

SearchBar::SearchBar(int x, int y, int w) : Widget(x, y, w, 1) {}

void SearchBar::render() const {
    Terminal::move_cursor(y_, x_);
    if (active_) Terminal::set_bold();
    std::string prefix = active_ ? "/ " : "  ";
    std::string line = prefix + buffer_;
    // Cursor indicator
    if (active_) line += "_";
    // Pad
    while (static_cast<int>(line.size()) < width_) line += ' ';
    if (static_cast<int>(line.size()) > width_) line = line.substr(0, width_);
    Terminal::write_str(line);
    if (active_) Terminal::reset_style();
}

void SearchBar::handle_key(int key) {
    if (!active_) return;
    if (key == Key::BACKSPACE && !buffer_.empty()) {
        buffer_.pop_back();
    } else if (key >= 32 && key < 127) {
        buffer_ += static_cast<char>(key);
    }
    if (on_change_) on_change_(buffer_);
}

void SearchBar::clear() {
    buffer_.clear();
    if (on_change_) on_change_(buffer_);
}
