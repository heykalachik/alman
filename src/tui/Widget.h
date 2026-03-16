#pragma once
#include "../Displayable.h"

class Widget : public Displayable {
protected:
    int x_, y_, width_, height_;
    bool focused_ = false;

public:
    Widget(int x, int y, int w, int h)
        : x_(x), y_(y), width_(w), height_(h) {}
    virtual ~Widget() = default;

    virtual void render() const = 0;
    virtual void handle_key(int key) = 0;
    virtual bool is_focusable() const { return true; }

    std::string display() const override;
    std::string summary() const override;

    void set_focused(bool f) { focused_ = f; }
    bool focused() const { return focused_; }
    int x() const { return x_; }
    int y() const { return y_; }
    int width() const { return width_; }
    int height() const { return height_; }
    void set_bounds(int x, int y, int w, int h) { x_=x; y_=y; width_=w; height_=h; }
};
