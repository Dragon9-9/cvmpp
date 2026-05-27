# Contributing to CVM++

Thank you for your interest in CVM++. This is primarily a personal open-source portfolio project; contributions are welcome.

## Getting started

1. Fork the repository and clone your fork.
2. Build: `make`
3. Run tests: `make verify`
4. Create a branch: `git checkout -b feature/your-change`

## Code style

- C++17, match existing naming and file layout (`include/cvm++/`, `src/`).
- Keep changes focused; avoid unrelated refactors.
- Compiler flags: `-Wall -Wextra -Wpedantic` (already in Makefile).

## Pull requests

- Ensure `make verify` passes.
- Update `examples/` if you add language features.
- Update documentation in `docs/guides/` and run `./scripts/build-docs.sh` if behavior or architecture changes.

## Reporting issues

Open a GitHub issue with:

- OS and compiler version
- Exact command run
- Expected vs actual behavior
- Minimal `.cvm` reproducer if applicable

## License

By contributing, you agree that your contributions will be licensed under the MIT License.
