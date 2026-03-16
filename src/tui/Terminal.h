#pragma once
#include <termios.h>
#include <string>

class Terminal {
    struct termios original_termios_;
    int width_  = 80;
    int height_ = 24;
    bool raw_mode_ = false;

public:
    Terminal();
    ~Terminal();

    void enter_raw_mode();
    void exit_raw_mode();
    void query_size();

    int width()  const { return width_; }
    int height() const { return height_; }

    // Returns key code: printable chars as-is; special keys as negative codes
    int read_key() const;

    static void clear_screen();
    static void move_cursor(int row, int col);
    static void hide_cursor();
    static void show_cursor();
    static void set_bold();
    static void set_dim();
    static void set_reverse();
    static void reset_style();
    static void set_fg(int color_code);   // ANSI 256-color
    static void write_str(const std::string& s);
};

// Special key codes (negative to avoid collision with printable chars)
namespace Key {
    constexpr int UP        = -1;
    constexpr int DOWN      = -2;
    constexpr int LEFT      = -3;
    constexpr int RIGHT     = -4;
    constexpr int ENTER     = '\r';
    constexpr int BACKSPACE = 127;
    constexpr int ESC       = 27;
    constexpr int CTRL_C    = 3;
    constexpr int CTRL_D    = 4;
}
