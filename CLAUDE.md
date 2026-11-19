# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project

Repository of the team "Intelligence naturelle" for algorithmic / puzzle code challenges ("Break the Code"). Code and comments are in C and French. There is no build system, test suite or linter: each piece is compiled by hand with `gcc`.

## Layout

- `training/<NN>-<name>/` — one directory per challenge, numbered by challenge ID. Each holds its input data (text, `.wav`, CSV) and its solver.
  - `17-hal/` — decode a 50 000-char lowercase text blob (`rahaha_le_code_est_la.txt`, single line, no newline); `halcode.c` is the driver.
  - `47-message/` — Morse code embedded in `message.wav`; `morse.c` is a self-contained decoder (Goertzel tone detection on a target frequency, 5 ms windows, run-length → dots/dashes). Needs PCM 16-bit WAV.
  - `80-graphs/` — graph/knapsack-style data: `Edges` (`departure,arrival,cost`) and `Items` (`id,planet,value,weight,type`), CSV without extension. The `*:Zone.Identifier` files are WSL/Windows download artifacts; ignore them.
- Repo root (`charge_fichier.c`, `fichier.h`) — shared helper code intended to be reused by challenge solvers. `fichier.h` declares `charger_texte(const char*)` (reads a whole file into a malloc'd buffer) and the `code` struct; `charge_fichier.c` implements it plus a work-in-progress `trouver_code`.

## Conventions

- **Compilation: each challenge directory has its own `Makefile`** (flags `-Wall -Wextra -g`; targets `all`, `run`, `clean`). Don't hand-type `gcc` lines, and there is no root Makefile. When adding a solver, create the `Makefile` in its challenge directory.
- **Documentation: every challenge directory has its own `README.md`** (data, approach, how to build/run, current state). Create or update it whenever you add or change a solver; don't document challenge specifics here.

## Commits

- Follow [Conventional Commits](https://www.conventionalcommits.org/): `type(scope): summary` (`feat`, `fix`, `docs`, `refactor`, `chore`, …). Use the challenge number as scope when relevant, e.g. `feat(17): ...`.
- Always add a body (description) after a blank line explaining what changed and why, not just a one-line subject.
- Do **not** add a `Co-Authored-By` line naming Claude, nor any "Generated with Claude Code" attribution, in commits or PR descriptions.

## Building / running

```
cd training/47-message && make run FREQ=700   # FREQ = tone frequency in Hz (default 700)
cd training/17-hal && make run                # currently fails to build, see below
make clean                                    # in either directory
```

Run `make` from inside the challenge directory: input file paths are relative. Solvers that need the shared helper (`17-hal`) reference `../../charge_fichier.c` and use `-I../..`.

## Known state / gotchas

- `charge_fichier.c` and `fichier.h` are mid-edit and **do not compile as is**: `trouver_code` has an empty `if()` and compares `char` to `NULL`; the `code` typedef lives in the .c file but is used in the header-visible API (move it to `fichier.h`); `fichier.h` has no include guard.
- `charger_texte` has bugs to be aware of before reusing it: the result is never NUL-terminated (the `realloc` size is `pos+temp` with no extra byte, and the initial 1-byte buffer is never terminated), `pos` advances by `TAILLE_TEXT` rather than the bytes actually read, and `assert(fichier != NULL)` makes the following NULL check dead code.
- Compiled binaries (e.g. `training/47-message/morse`) are build artifacts, not source; don't commit them.
