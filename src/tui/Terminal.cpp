#include "Terminal.h"
#include <unistd.h>
#include <sys/ioctl.h>
#include <cstdio>
#include <cstring>
#include <stdexcept>

Terminal::Terminal() {
    query_size();
}

Terminal::~Terminal() {
    if (raw_mode_) exit_raw_mode();
    show_cursor();
}

void Terminal::enter_raw_mode() {
    if (tcgetattr(STDIN_FILENO, &original_termios_) == -1)
        throw std::runtime_error("tcgetattr failed");

    struct termios raw = original_termios_;
    raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
    raw.c_oflag &= ~(OPOST);
    raw.c_cflag |=  (CS8);
    raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
    raw.c_cc[VMIN]  = 1;
    raw.c_cc[VTIME] = 0;

    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1)
        throw std::runtime_error("tcsetattr failed");
    raw_mode_ = true;
    hide_cursor();
}

void Terminal::exit_raw_mode() {
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &original_termios_);
    raw_mode_ = false;
}

void Terminal::query_size() {
    struct winsize ws{};
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == 0 && ws.ws_col > 0) {
        width_  = ws.ws_col;
        height_ = ws.ws_row;
    }
}

int Terminal::read_key() const {
    char c = 0;
    if (::read(STDIN_FILENO, &c, 1) <= 0) return Key::ESC;

    if (c == Key::ESC) {
        char seq[3] = {};
        if (::read(STDIN_FILENO, &seq[0], 1) <= 0) return Key::ESC;
        if (::read(STDIN_FILENO, &seq[1], 1) <= 0) return Key::ESC;
        if (seq[0] == '[') {
            switch (seq[1]) {
                case 'A': return Key::UP;
                case 'B': return Key::DOWN;
                case 'C': return Key::RIGHT;
                case 'D': return Key::LEFT;
            }
        }
        return Key::ESC;
    }
    return static_cast<int>(static_cast<unsigned char>(c));
}

void Terminal::clear_screen() {
    write(STDOUT_FILENO, "\x1b[2J\x1b[H", 7);
}

void Terminal::move_cursor(int row, int col) {
    char buf[32];
    int n = snprintf(buf, sizeof(buf), "\x1b[%d;%dH", row, col);
    write(STDOUT_FILENO, buf, n);
}

void Terminal::hide_cursor() { write(STDOUT_FILENO, "\x1b[?25l", 6); }
void Terminal::show_cursor() { write(STDOUT_FILENO, "\x1b[?25h", 6); }
void Terminal::set_bold()    { write(STDOUT_FILENO, "\x1b[1m", 4); }
void Terminal::set_dim()     { write(STDOUT_FILENO, "\x1b[2m", 4); }
void Terminal::set_reverse() { write(STDOUT_FILENO, "\x1b[7m", 4); }
void Terminal::reset_style() { write(STDOUT_FILENO, "\x1b[0m", 4); }

void Terminal::set_fg(int color_code) {
    char buf[16];
    int n = snprintf(buf, sizeof(buf), "\x1b[38;5;%dm", color_code);
    write(STDOUT_FILENO, buf, n);
}

void Terminal::write_str(const std::string& s) {
    write(STDOUT_FILENO, s.c_str(), s.size());
}
