# VCS (Version Control System) - Project Outline

This repository is an in-progress implementation outline for a custom VCS written in C++.

## Project Goal

Build a lightweight Git-inspired VCS to better understand core concepts:

- Object storage (blob/tree/commit-like structures)
- Content-addressing and hashing
- Local configuration
- Staging/index behavior
- Commit history traversal
- Basic branching and checkout semantics

## Planned Command Surface (Draft)

- [x] `vcs init`
- [x] `vcs config <user.name|user.email>`
- [x] `vcs add <path>`
- [ ] `vcs commit -m "message"`
- [x] `vcs status`
- [ ] `vcs log`
- [ ] `vcs branch <name>`
- [ ] `vcs checkout <name|commit>`

This list is intentionally tentative and may evolve as internals are implemented.

## High-Level Design (Draft)

### Repository Layout

```text
.vcs/
  objects/
  refs/
    heads/
  HEAD
  index
  config
```

### Core Components (Planned)

- `hashing`: object IDs and content hashing
- `compress`: compress data with Huffman coding
- `config`: configuring the local vcs instance
- `io`: input and logging utilities 
- `head`: refs, branches, and HEAD
- `index`: staging area format + updates
- `commits`: commit creation and parent links
- `commands`: directory for command implementations (init, add, commit, etc.)

## Build and Run

```bash
cmake -S . -B build
cmake --build build
./build/vcs
```

## Notes

This project is intended primarily as a learning experience, through which I can better understand the inner workings of version control systems like Git. 

Not all features will be implemented, and some may be simplified or omitted for educational purposes. This is never intended to be a production-ready VCS, but rather a sandbox for exploring the concepts and mechanics behind version control.

While C would be ideal for making a professional VCS, C++ is chosen here for its balance of performance and ease of use, especially for rapid prototyping and experimentation, though some lower-level pointer manipulation was required, especially with compression and hashing.