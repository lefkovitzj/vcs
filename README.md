# VCS (Version Control System) - Project Outline

This repository is an in-progress implementation outline for a custom VCS written in C++.

## Status

- Early scaffold stage
- Architecture and feature scope are subject to change
- Current executable is a bootstrap placeholder

## Project Goal

Build a lightweight Git-inspired VCS to better understand core concepts:

- Object storage (blob/tree/commit-like structures)
- Content-addressing and hashing
- Staging/index behavior
- Commit history traversal
- Basic branching and checkout semantics

## Planned Command Surface (Draft)

- `vcs init`
- `vcs add <path>`
- `vcs commit -m "message"`
- `vcs status`
- `vcs log`
- `vcs branch <name>`
- `vcs checkout <name|commit>`

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
```

### Core Modules (Planned)

- `hashing`: object IDs and content hashing
- `objects`: read/write serialized objects
- `index`: staging area format + updates
- `commits`: commit creation and parent links
- `refs`: branches and HEAD resolution
- `commands`: CLI command handlers

## Current Workspace

The project currently contains a minimal CMake + C++ scaffold:

- `CMakeLists.txt`
- `main.cpp`

## Build and Run

```bash
cmake -S . -B build
cmake --build build
./build/vcs
```

## Near-Term Milestones

- [ ] Replace placeholder `main.cpp` with CLI argument parsing
- [ ] Implement `vcs init` to create `.vcs` directory structure
- [ ] Add object hashing + write/read roundtrip
- [ ] Implement basic `add` staging flow
- [ ] Implement first working `commit`

## Notes

This README is a living outline and will be updated as architecture decisions solidify.