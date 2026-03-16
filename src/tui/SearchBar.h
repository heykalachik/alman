#pragma once
#include "Widget.h"
#include <string>
#include <functional>

class SearchBar : public Widget {
    std::string buffer_;
    std::function<void(const std::string&)> on_change_;
    bool active_ = false;

public:
    SearchBar(int x, int y, int w);

    void render() const override;
    void handle_key(int key) override;
    bool is_focusable() const override { return true; }

    const std::string& query() const { return buffer_; }
    void set_on_change(std::function<void(const std::string&)> cb) { on_change_ = std::move(cb); }
    void clear();
    void set_active(bool a) { active_ = a; }
    bool active() const { return active_; }
};
