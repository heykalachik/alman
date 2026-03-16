#include "ListView.h"
#include "Terminal.h"
#include <algorithm>
#include <string>

ListView::ListView(int x, int y, int w, int h) : Widget(x, y, w, h) {}

void ListView::set_items(std::vector<Displayable*> items, std::vector<std::string> labels) {
    items_ = std::move(items);
    group_labels_ = std::move(labels);
    if (selected_ >= items_.size()) selected_ = items_.empty() ? 0 : items_.size() - 1;
    if (scroll_ > selected_) scroll_ = selected_;
}

Displayable* ListView::selected_item() const {
    if (items_.empty()) return nullptr;
    return items_[selected_];
}

void ListView::render() const {
    size_t visible = static_cast<size_t>(height_);

    std::for_each(items_.begin(), items_.end(), [&](Displayable* item) {
        size_t idx = static_cast<size_t>(item - items_[0]) / sizeof(Displayable*);
        // We'll do indexed loop below instead
        (void)item;
    });

    for (int row = 0; row < height_; ++row) {
        size_t idx = scroll_ + static_cast<size_t>(row);
        Terminal::move_cursor(y_ + row, x_);

        if (idx >= items_.size()) {
            // Empty row: pad with spaces
            std::string blank(width_, ' ');
            Terminal::write_str(blank);
            continue;
        }

        bool is_selected = (idx == selected_);

        if (is_selected) Terminal::set_reverse();

        // Build line
        std::string label;
        if (idx < group_labels_.size() && !group_labels_[idx].empty())
            label = "[" + group_labels_[idx] + "] ";

        std::string text = label + items_[idx]->summary();
        if (static_cast<int>(text.size()) > width_)
            text = text.substr(0, width_ - 1) + "…";

        // Pad to fill width
        while (static_cast<int>(text.size()) < width_)
            text += ' ';

        Terminal::write_str(text);

        if (is_selected) Terminal::reset_style();
    }
    (void)visible;
}

void ListView::handle_key(int key) {
    if (key == Key::UP || key == 'k') select_prev();
    else if (key == Key::DOWN || key == 'j') select_next();
}

void ListView::select_next() {
    if (items_.empty()) return;
    if (selected_ + 1 < items_.size()) {
        ++selected_;
        if (selected_ >= scroll_ + static_cast<size_t>(height_))
            ++scroll_;
    }
}

void ListView::select_prev() {
    if (items_.empty() || selected_ == 0) return;
    --selected_;
    if (selected_ < scroll_) scroll_ = selected_;
}

void ListView::select_index(size_t idx) {
    if (idx >= items_.size()) return;
    selected_ = idx;
    if (selected_ < scroll_) scroll_ = selected_;
    if (selected_ >= scroll_ + static_cast<size_t>(height_))
        scroll_ = selected_ - static_cast<size_t>(height_) + 1;
}
