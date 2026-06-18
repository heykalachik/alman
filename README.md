# alman

Terminal UI for managing shell aliases. Changes are written to `~/.zshrc` instantly.

## Build

```sh
cmake -S . -B build && cmake --build build
./build/alman
```

Requires C++20 and CMake 4.1+.

## Usage

| Key | Action |
|-----|--------|
| `a` | Add alias |
| `e` | Edit selected |
| `d` | Delete selected |
| `/` | Search |
| `Enter` / `l` | View detail |
| `x` | Force re-export |
| `q` | Quit |

Aliases are stored in `~/.config/alman/aliases.alman` and synced to `~/.zshrc` on every change.

## Requirements

### 1. Minimum 6 classes, at least one abstract

| Class | File | Abstract |
|-------|------|----------|
| `Displayable` | `src/Displayable.h` | yes — `display()`, `summary()` are pure virtual |
| `ShellExporter` | `src/ShellExporter.h` | yes — `format_alias()`, `header()`, `shell_name()` are pure virtual |
| `Widget` | `src/tui/Widget.h` | yes — `render()`, `handle_key()` are pure virtual |
| `Alias` | `src/Alias.h` | no |
| `AliasGroup` | `src/AliasGroup.h` | no |
| `AliasStore` | `src/AliasStore.h` | no |
| `ZshExporter` | `src/ZshExporter.h` | no |
| `BashExporter` | `src/BashExporter.h` | no |
| `Terminal` | `src/tui/Terminal.h` | no |
| `ListView` | `src/tui/ListView.h` | no |
| `SearchBar` | `src/tui/SearchBar.h` | no |
| `App` | `src/tui/App.h` | no |

Plus 5 exception classes in `src/exceptions.h`.

### 2. Inheritance

```
Displayable  (abstract)
├── Alias
├── AliasGroup
└── Widget  (abstract)
    ├── ListView
    └── SearchBar

ShellExporter  (abstract)
├── ZshExporter
└── BashExporter

std::runtime_error
└── AlmanException
    ├── AliasStoreException
    ├── DuplicateAliasException
    ├── NotFoundException
    └── InvalidAliasException
```

### 3. Polymorphism

- `src/tui/App.h` — `std::unique_ptr<ShellExporter> exporter_` holds either `ZshExporter` or `BashExporter`; all calls go through the base pointer.
- `src/tui/ListView.h` — `std::vector<Displayable*> items_` stores `Alias*` and `AliasGroup*` together.
- `src/main.cpp` — `demo_polymorphism()` builds both vectors and calls virtual methods on them.

### 4. Virtual functions and object manipulation via pointers

| Virtual method | Declared in | Overridden in |
|----------------|-------------|---------------|
| `display()` | `Displayable` | `Alias`, `AliasGroup`, `Widget` |
| `summary()` | `Displayable` | `Alias`, `AliasGroup`, `Widget` |
| `print()` | `Displayable` | (default used; called via `Displayable*`) |
| `render()` | `Widget` | `ListView`, `SearchBar` |
| `handle_key()` | `Widget` | `ListView`, `SearchBar` |
| `format_alias()` | `ShellExporter` | `ZshExporter`, `BashExporter` |
| `header()` | `ShellExporter` | `ZshExporter`, `BashExporter` |
| `shell_name()` | `ShellExporter` | `ZshExporter`, `BashExporter` |

Demonstrated in `src/main.cpp` `demo_polymorphism()`: a `std::vector<Displayable*>` holds mixed types, and `d->print()` dispatches correctly via vtable.

### 5. Operator overloads (≥ 2)

| Operator | Class | File | Purpose |
|----------|-------|------|---------|
| `operator==` | `Alias` | `src/Alias.h:26` | equality by name |
| `operator<` | `Alias` | `src/Alias.h:27` | name-based ordering (used by `std::sort`) |
| `operator<<` | `Alias` (friend) | `src/Alias.h:28` | stream output |
| `operator+=` | `AliasGroup` | `src/AliasGroup.h:34` | add alias to group |
| `operator==` | `AliasGroup` | `src/AliasGroup.h:35` | equality by name |

### 6. Algorithms from `<algorithm>` (≥ 3)

| Algorithm | Location | Purpose |
|-----------|----------|---------|
| `std::find_if` | `src/AliasGroup.cpp:16,24,32`, `src/AliasStore.cpp:19,27,33` | locate alias/group by name |
| `std::copy_if` | `src/AliasGroup.cpp:40` | filter aliases matching a search query |
| `std::sort` | `src/AliasGroup.cpp:50`, `src/AliasStore.cpp:48` | sort by name via `operator<` |
| `std::any_of` | `src/AliasGroup.cpp:54`, `src/AliasStore.cpp:12` | check for duplicate names |
| `std::for_each` | `src/AliasGroup.cpp:63`, `src/ShellExporter.cpp:10`, `src/main.cpp:56,60`, `src/tui/ListView.cpp:23` | iterate and apply operations |
| `std::accumulate` | `src/AliasStore.cpp:53` | sum alias counts across all groups |
| `std::transform` | `src/tui/App.cpp:84` | convert `Alias` vector to `Displayable*` vector |

### 7. Exceptions

Custom hierarchy rooted at `AlmanException` (`src/exceptions.h`):

- `AliasStoreException` — thrown on file I/O failure (`src/AliasStore.cpp`)
- `DuplicateAliasException` — thrown when adding an alias that already exists (`src/AliasGroup.cpp`)
- `NotFoundException` — thrown when an alias or group is not found (`src/AliasGroup.cpp`, `src/AliasStore.cpp`)
- `InvalidAliasException` — thrown in `Alias` constructor for empty or whitespace-containing names (`src/Alias.cpp`)

Top-level catch in `src/main.cpp`; per-operation catches in `src/tui/App.cpp`.

### 8. Reading and writing files

| Operation | File | Location |
|-----------|------|----------|
| Read aliases store | `src/AliasStore.cpp` | `AliasStore::load()` — reads `~/.config/alman/aliases.alman` line by line with `std::ifstream` |
| Write aliases store | `src/AliasStore.cpp` | `AliasStore::save()` — overwrites the same file with `std::ofstream` |
| Read existing shell config | `src/ShellExporter.cpp` | `ShellExporter::export_all()` — reads `~/.zshrc` / `~/.bashrc` to preserve user content |
| Write shell config | `src/ShellExporter.cpp` | appends the managed alias block back to the shell config |
| Backup shell config | `src/ShellExporter.cpp` | copies `~/.zshrc` → `~/.zshrc.alman_backup` via `std::filesystem::copy_file` |

### 9. Vector of objects with use of class methods

`src/main.cpp` `demo_polymorphism()`:

- `std::vector<Alias>` — creates aliases, calls `display()`, `summary()`, `print()`, `operator==`, `operator<`, `operator<<`
- `std::vector<AliasGroup>` — creates groups, calls `add()`, `remove()`, `search()`, `sort_by_name()`, `contains()`, `operator+=`
- `std::vector<Displayable*>` — polymorphic vector, calls `print()` and `display()` via base pointer
- `std::vector<std::unique_ptr<ShellExporter>>` — holds `ZshExporter` and `BashExporter`, calls `shell_name()` and `format_alias()` polymorphically

---

```
┌─────────────────────────────────────────────────────┐
│                   main.cpp                          │  Entery point
├─────────────────────────────────────────────────────┤
│                   App (tui/App)                     │  
│   ┌─────────────┐  ┌──────────┐  ┌───────────────┐  │
│   │  Terminal   │  │ ListView │  │   SearchBar   │  │  TUI layer
│   │  (tui/)     │  │  (tui/)  │  │    (tui/)     │  │
│   └─────────────┘  └──────────┘  └───────────────┘  │
│         ^                ^                          │
│      Widget (tui/)  <  Displayable.h  (interface)   │
├─────────────────────────────────────────────────────┤
│   AliasStore  <  AliasGroup  <  Alias               │  Data layer
├─────────────────────────────────────────────────────┤
│   ShellExporter > ZshExporter / BashExporter        │  Export layer
├─────────────────────────────────────────────────────┤
│   exceptions.h                                      │  Exceptions
└─────────────────────────────────────────────────────┘
```
