# Change Log

All notable changes to the "bateman-syntax" extension will be documented in this file.

Check [Keep a Changelog](http://keepachangelog.com/) for recommendations on how to structure this file.

## [0.0.4]

- Fix `Do you like Huey Lewis and the News?` highlighting (drop broken trailing `\\b` after `?`).
- Move `I live in the American Gardens Building` earlier; match without word boundaries.
- Use `entity.name.function.bateman` for names after function declarations and calls.

## [0.0.3]

- Restore original flat grammar and scope names from extension2 (fixes theme colors).
- Add else/while/break phrases and fix comment matching for current lexer.
- Highlight function names before `(` with `variable.language.bateman`.

## [0.0.2]

- Align syntax grammar with current `src/lexer.l` (comments, `This is not an exit`, else/while phrases, call vs print).

## [0.0.1]

- Initial release