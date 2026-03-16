#include "App.h"
#include "../ZshExporter.h"
#include "../BashExporter.h"
#include "../exceptions.h"
#include <algorithm>
#include <cstdlib>
#include <string>

// ── helpers ──────────────────────────────────────────────────────────────────

static std::string store_default_path() {
    const char* home = std::getenv("HOME");
    std::string h = home ? home : ".";
    return h + "/.config/alman/aliases.alman";
}

// ── App ──────────────────────────────────────────────────────────────────────

App::App(std::string store_path)
    : store_(store_path.empty() ? store_default_path() : store_path),
      list_view_(1, 4, 80, 20),
      search_bar_(1, 3, 80)
{
    const char* home = std::getenv("HOME");
    std::string zshrc = home ? std::string(home) + "/.zshrc" : ".zshrc";
    exporter_ = std::make_unique<ZshExporter>(zshrc);
}

void App::setup() {
    terminal_.query_size();
    int W = terminal_.width();
    int H = terminal_.height();
    list_view_.set_bounds(1, 4, W, H - 5);
    search_bar_.set_bounds(1, 3, W, 1);

    search_bar_.set_on_change([this](const std::string& q) { refresh_list(q); });

    try { store_.load(); }
    catch (const AliasStoreException&) { /* fresh start */ }

    // Seed with sample data if completely empty
    if (store_.groups().empty()) {
        AliasGroup git("git", "Git shortcuts");
        git += Alias("gs",  "git status",           "Show repo status");
        git += Alias("gp",  "git push",              "Push to remote");
        git += Alias("gc",  "git commit -m",         "Commit with message");
        git += Alias("gl",  "git log --oneline -10", "Short log");
        store_.add_group(std::move(git));

        AliasGroup nav("navigation", "Directory shortcuts");
        nav += Alias("..",  "cd ..",  "Go up one level");
        nav += Alias("...", "cd ../..", "Go up two levels");
        nav += Alias("~",   "cd ~",   "Go to home");
        store_.add_group(std::move(nav));

        store_.save();
    }

    refresh_list();
}

void App::refresh_list(const std::string& query) {
    flat_aliases_.clear();
    flat_labels_.clear();

    if (query.empty()) {
        for (const auto& group : store_.groups()) {
            for (const auto& alias : group.aliases()) {
                flat_aliases_.push_back(alias);
                flat_labels_.push_back(group.name());
            }
        }
    } else {
        for (const auto& group : store_.groups()) {
            for (const auto& alias : group.search(query)) {
                flat_aliases_.push_back(alias);
                flat_labels_.push_back(group.name());
            }
        }
    }

    display_ptrs_.clear();
    display_ptrs_.reserve(flat_aliases_.size());
    std::transform(flat_aliases_.begin(), flat_aliases_.end(),
        std::back_inserter(display_ptrs_),
        [](Alias& a) -> Displayable* { return &a; });

    list_view_.set_items(display_ptrs_, flat_labels_);
}

// ── render ────────────────────────────────────────────────────────────────────

void App::render_header() {
    int W = terminal_.width();
    Terminal::move_cursor(1, 1);
    Terminal::set_bold();
    Terminal::set_fg(39); // bright blue
    std::string title = "  alman — alias manager";
    std::string right = "[q:quit  a:add  e:edit  d:del  /:search  x:force-export]  ";
    // Pad between title and right
    int pad = W - static_cast<int>(title.size()) - static_cast<int>(right.size());
    if (pad < 0) pad = 0;
    Terminal::write_str(title + std::string(pad, ' ') + right);
    Terminal::reset_style();

    // Divider
    Terminal::move_cursor(2, 1);
    Terminal::set_dim();
    Terminal::write_str(std::string(W, '-'));
    Terminal::reset_style();
}

void App::render_status() {
    int W = terminal_.width();
    int H = terminal_.height();
    Terminal::move_cursor(H - 1, 1);
    Terminal::set_dim();
    std::string msg = status_msg_.empty()
        ? " " + std::to_string(store_.total_alias_count()) + " aliases in "
          + std::to_string(store_.groups().size()) + " groups"
        : " " + status_msg_;
    if (static_cast<int>(msg.size()) > W) msg = msg.substr(0, W);
    while (static_cast<int>(msg.size()) < W) msg += ' ';
    Terminal::write_str(msg);
    Terminal::reset_style();
    status_msg_.clear();
}

void App::render_detail() {
    int W = terminal_.width();
    int H = terminal_.height();
    Alias* a = selected_alias();
    if (!a) return;

    int row = 4;
    auto line = [&](const std::string& label, const std::string& val) {
        Terminal::move_cursor(row++, 2);
        Terminal::set_bold(); Terminal::write_str(label + ": "); Terminal::reset_style();
        Terminal::write_str(val);
        Terminal::write_str(std::string(W - 2 - label.size() - 2 - val.size(), ' '));
    };
    Terminal::move_cursor(3, 1);
    Terminal::set_bold(); Terminal::set_fg(39);
    std::string hdr = "  Detail — press ESC to go back";
    Terminal::write_str(hdr + std::string(W - hdr.size(), ' '));
    Terminal::reset_style();
    ++row;

    line("Name",        a->name());
    line("Command",     a->command());
    line("Description", a->description().empty() ? "(none)" : a->description());
    line("Created",     a->created_at());

    ++row;
    Terminal::move_cursor(row++, 2);
    Terminal::set_bold(); Terminal::write_str("Shell line:"); Terminal::reset_style();
    Terminal::move_cursor(row++, 2);
    Terminal::set_fg(40); // green
    Terminal::write_str(a->display());
    Terminal::reset_style();

    // Clear remaining lines
    for (; row < H - 2; ++row) {
        Terminal::move_cursor(row, 1);
        Terminal::write_str(std::string(W, ' '));
    }
    (void)H;
}

void App::render_add_edit() {
    int W = terminal_.width();
    Terminal::move_cursor(3, 1);
    Terminal::set_bold(); Terminal::set_fg(220); // yellow
    std::string hdr = (mode_ == AppMode::ADD) ? "  Add Alias" : "  Edit Alias";
    hdr += " — TAB: next field  ENTER: confirm  ESC: cancel";
    Terminal::write_str(hdr + std::string(W - static_cast<int>(hdr.size()), ' '));
    Terminal::reset_style();

    auto field = [&](int row, const std::string& label, const std::string& val, int field_idx) {
        Terminal::move_cursor(row, 2);
        if (input_field_ == field_idx) Terminal::set_reverse();
        std::string line = label + ": " + val + "_";
        while (static_cast<int>(line.size()) < W - 2) line += ' ';
        Terminal::write_str(line);
        Terminal::reset_style();
    };

    field(5,  "Group      ", input_group_,       0);
    field(7,  "Name       ", input_name_,         1);
    field(9,  "Command    ", input_command_,      2);
    field(11, "Description", input_description_,  3);
}

void App::render_confirm_delete() {
    int W = terminal_.width();
    Alias* a = selected_alias();
    if (!a) return;
    Terminal::move_cursor(3, 1);
    Terminal::set_bold(); Terminal::set_fg(196); // red
    std::string hdr = "  Delete '" + a->name() + "'?  [y] yes   [n] cancel";
    Terminal::write_str(hdr + std::string(W - static_cast<int>(hdr.size()), ' '));
    Terminal::reset_style();
}

void App::render() {
    Terminal::clear_screen();
    render_header();

    switch (mode_) {
        case AppMode::LIST:
            search_bar_.render();
            list_view_.render();
            break;
        case AppMode::SEARCH:
            search_bar_.render();
            list_view_.render();
            break;
        case AppMode::DETAIL:
            render_detail();
            break;
        case AppMode::ADD:
        case AppMode::EDIT:
            render_add_edit();
            break;
        case AppMode::CONFIRM_DELETE:
            render_confirm_delete();
            break;
    }

    render_status();
}

// ── input ─────────────────────────────────────────────────────────────────────

Alias* App::selected_alias() {
    size_t idx = list_view_.selected_index();
    if (idx >= flat_aliases_.size()) return nullptr;
    return &flat_aliases_[idx];
}

void App::handle_list_key(int key) {
    switch (key) {
        case 'q': case Key::CTRL_C: case Key::CTRL_D:
            running_ = false; break;
        case 'j': case Key::DOWN:
            list_view_.handle_key(key); break;
        case 'k': case Key::UP:
            list_view_.handle_key(key); break;
        case Key::ENTER: case 'l':
            mode_ = AppMode::DETAIL; break;
        case '/':
            search_bar_.set_active(true);
            mode_ = AppMode::SEARCH; break;
        case 'a':
            enter_add_mode(); break;
        case 'e':
            if (selected_alias()) enter_edit_mode(); break;
        case 'd':
            if (selected_alias()) mode_ = AppMode::CONFIRM_DELETE; break;
        case 'x':
            do_export(); break;
        case 'S':
            store_.sort_groups(); refresh_list(search_bar_.query()); break;
        default: break;
    }
}

void App::handle_search_key(int key) {
    if (key == Key::ESC || key == Key::ENTER) {
        search_bar_.set_active(false);
        mode_ = AppMode::LIST;
        return;
    }
    if (key == Key::DOWN || key == 'j') {
        list_view_.handle_key(Key::DOWN);
        return;
    }
    if (key == Key::UP || key == 'k') {
        list_view_.handle_key(Key::UP);
        return;
    }
    search_bar_.handle_key(key);
}

static std::string& active_field(int idx, std::string& g, std::string& n,
                                  std::string& c, std::string& d) {
    static std::string dummy;
    switch(idx) { case 0: return g; case 1: return n; case 2: return c; case 3: return d; }
    return dummy;
}

void App::handle_add_edit_key(int key) {
    if (key == Key::ESC) { mode_ = AppMode::LIST; return; }
    if (key == Key::ENTER) {
        if (mode_ == AppMode::ADD) commit_add();
        else commit_edit();
        return;
    }
    if (key == '\t' || key == Key::DOWN) {
        input_field_ = (input_field_ + 1) % 4; return;
    }
    if (key == Key::UP) {
        input_field_ = (input_field_ + 3) % 4; return;
    }
    auto& field = active_field(input_field_, input_group_, input_name_, input_command_, input_description_);
    if (key == Key::BACKSPACE && !field.empty()) field.pop_back();
    else if (key >= 32 && key < 127) field += static_cast<char>(key);
}

void App::handle_confirm_delete_key(int key) {
    if (key == 'y' || key == 'Y') commit_delete();
    else mode_ = AppMode::LIST;
}

void App::handle_input(int key) {
    switch (mode_) {
        case AppMode::LIST:             handle_list_key(key); break;
        case AppMode::SEARCH:           handle_search_key(key); break;
        case AppMode::DETAIL:
            if (key == Key::ESC || key == 'h' || key == 'q') mode_ = AppMode::LIST;
            break;
        case AppMode::ADD: case AppMode::EDIT: handle_add_edit_key(key); break;
        case AppMode::CONFIRM_DELETE:   handle_confirm_delete_key(key); break;
    }
}

// ── CRUD operations ───────────────────────────────────────────────────────────

void App::enter_add_mode() {
    input_group_       = store_.groups().empty() ? "general" : store_.groups()[0].name();
    input_name_        = "";
    input_command_     = "";
    input_description_ = "";
    input_field_       = 1;
    mode_              = AppMode::ADD;
}

void App::enter_edit_mode() {
    Alias* a = selected_alias();
    if (!a) return;
    size_t idx = list_view_.selected_index();
    input_group_       = (idx < flat_labels_.size()) ? flat_labels_[idx] : "";
    input_name_        = a->name();
    input_command_     = a->command();
    input_description_ = a->description();
    input_field_       = 2;
    mode_              = AppMode::EDIT;
}

void App::commit_add() {
    if (input_name_.empty() || input_command_.empty()) {
        set_status("Name and command are required.");
        return;
    }
    try {
        AliasGroup* group = store_.find_group(input_group_);
        if (!group) {
            store_.add_group(AliasGroup(input_group_));
            group = store_.find_group(input_group_);
        }
        group->add(Alias(input_name_, input_command_, input_description_));
        store_.save();
        exporter_->export_all(store_);
        refresh_list(search_bar_.query());
        set_status("Added '" + input_name_ + "' and saved to " + exporter_->target_path() + ".");
        mode_ = AppMode::LIST;
    } catch (const AlmanException& e) {
        set_status(e.what());
    }
}

void App::commit_edit() {
    if (input_command_.empty()) {
        set_status("Command cannot be empty.");
        return;
    }
    try {
        AliasGroup* group = store_.find_group(input_group_);
        if (!group) { set_status("Group not found."); return; }
        group->update(input_name_, input_command_);
        // Update description via find + set
        auto opt = group->find(input_name_);
        if (opt) {
            group->remove(input_name_);
            Alias updated(input_name_, input_command_, input_description_);
            group->add(std::move(updated));
        }
        store_.save();
        exporter_->export_all(store_);
        refresh_list(search_bar_.query());
        set_status("Updated '" + input_name_ + "' and saved to " + exporter_->target_path() + ".");
        mode_ = AppMode::LIST;
    } catch (const AlmanException& e) {
        set_status(e.what());
    }
}

void App::commit_delete() {
    Alias* a = selected_alias();
    if (!a) { mode_ = AppMode::LIST; return; }
    size_t idx = list_view_.selected_index();
    std::string group_name = (idx < flat_labels_.size()) ? flat_labels_[idx] : "";
    std::string alias_name = a->name();
    try {
        AliasGroup* group = store_.find_group(group_name);
        if (group) group->remove(alias_name);
        store_.save();
        exporter_->export_all(store_);
        refresh_list(search_bar_.query());
        set_status("Deleted '" + alias_name + "' and saved to " + exporter_->target_path() + ".");
    } catch (const AlmanException& e) {
        set_status(e.what());
    }
    mode_ = AppMode::LIST;
}

void App::do_export() {
    try {
        exporter_->export_all(store_);
        set_status("Exported to " + exporter_->target_path() + ". Restart your shell to apply.");
    } catch (const AlmanException& e) {
        set_status(std::string("Export failed: ") + e.what());
    }
}

void App::set_status(const std::string& msg) {
    status_msg_ = msg;
}

// ── run ───────────────────────────────────────────────────────────────────────

void App::run() {
    setup();
    terminal_.enter_raw_mode();
    running_ = true;
    while (running_) {
        terminal_.query_size();
        int W = terminal_.width();
        int H = terminal_.height();
        list_view_.set_bounds(1, 4, W, H - 5);
        search_bar_.set_bounds(1, 3, W, 1);
        render();
        int key = terminal_.read_key();
        handle_input(key);
    }
    terminal_.exit_raw_mode();
    Terminal::clear_screen();
    Terminal::move_cursor(1, 1);
    Terminal::show_cursor();
}
