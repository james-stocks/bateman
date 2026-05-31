# Contributing

## VS Code syntax extension

Syntax highlighting for `.bateman` files is in `vscode/bateman-syntax/`.

Install it locally: **Extensions: Install Extension from Location...** → select `vscode/bateman-syntax/`.

If you change the language in `src/lexer.l` or `src/parser.y`, update the extension grammar in `vscode/bateman-syntax/syntaxes/bateman.tmLanguage.json` (and `language-configuration.json` when comment or bracket rules change).

## Compiler

Build and test changes with:

```bash
make test
```

See `AGENTS.md` for full environment setup on a fresh machine.
