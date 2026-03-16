#pragma once
#include "Widget.h"
#include "../Displayable.h"
#include <vector>
#include <string>

class ListView : public Widget {
    std::vector<Displayable*> items_;
    size_t selected_ = 0;
    size_t scroll_   = 0;
    std::vector<std::string> group_labels_; // optional label per item (group name)

public:
    ListView(int x, int y, int w, int h);

    void set_items(std::vector<Displayable*> items, std::vector<std::string> labels = {});
    Displayable* selected_item() const;
    size_t selected_index() const { return selected_; }
    size_t item_count() const { return items_.size(); }

    void render() const override;
    void handle_key(int key) override;

    void select_next();
    void select_prev();
    void select_index(size_t idx);
};
