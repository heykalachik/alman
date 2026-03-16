#pragma once
#include "../AliasStore.h"
#include "../ShellExporter.h"
#include "Terminal.h"
#include "ListView.h"
#include "SearchBar.h"
#include <memory>
#include <string>
#include <vector>

enum class AppMode { LIST, SEARCH, DETAIL, ADD, EDIT, CONFIRM_DELETE };

class App {
    Terminal terminal_;
    AliasStore store_;
    ListView list_view_;
    SearchBar search_bar_;
    std::unique_ptr<ShellExporter> exporter_;

    bool running_ = false;
    AppMode mode_ = AppMode::LIST;

    // For ADD/EDIT input
    std::string input_group_;
    std::string input_name_;
    std::string input_command_;
    std::string input_description_;
    int         input_field_ = 0; // which field is active

    // Flat list of aliases (for list view)
    std::vector<Alias>       flat_aliases_;
    std::vector<std::string> flat_labels_;
    std::vector<Displayable*> display_ptrs_;

    std::string status_msg_;

public:
    explicit App(std::string store_path);
    void run();

private:
    void setup();
    void render();
    void render_header();
    void render_status();
    void render_detail();
    void render_add_edit();
    void render_confirm_delete();

    void handle_input(int key);
    void handle_list_key(int key);
    void handle_search_key(int key);
    void handle_add_edit_key(int key);
    void handle_confirm_delete_key(int key);

    void refresh_list(const std::string& query = "");
    void enter_add_mode();
    void enter_edit_mode();
    void commit_add();
    void commit_edit();
    void commit_delete();
    void do_export();

    Alias* selected_alias();
    void set_status(const std::string& msg);
};
