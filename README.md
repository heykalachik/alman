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
