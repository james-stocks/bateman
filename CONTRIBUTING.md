# Contributing

## VS Code syntax extension

Syntax highlighting for `.bateman` files is in `vscode/bateman-syntax/`.

Install it locally: **Developer: Install Extension from Location...** → select `vscode/bateman-syntax/`.

If you change the language in `src/lexer.l` or `src/parser.y`, update the extension grammar in `vscode/bateman-syntax/syntaxes/bateman.tmLanguage.json` (and `language-configuration.json` when comment or bracket rules change).

## Compiler

Build and test changes with:

```bash
make test
```

See `AGENTS.md` for full environment setup on a fresh machine.

## Versioning

The language and compiler version live in [`VERSION`](VERSION) at the repo root (semver, currently `0.x` before 1.0).

- Bump `VERSION` in pull requests when you make user-visible language or compiler changes.
- After merge to `main`, [`.github/workflows/release-tag.yaml`](.github/workflows/release-tag.yaml) creates an annotated git tag `v<VERSION>` if that tag does not already exist.
- Keep [`vscode/bateman-syntax/package.json`](vscode/bateman-syntax/package.json) aligned when grammar changes.
- Check the compiler version with `build/bateman --version`.
