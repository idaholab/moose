
# AbaqusInputParser

## Overview

- Purpose: Parse Abaqus `.inp` text into a light, lossless syntax tree that preserves block/option structure, headers, and data lines for later semantic processing.
- Scope: Tokenization, header parsing, continuation handling, and basic validation (unknown keywords). No semantic interpretation of nodes, elements, sets, or instances.

## Key Types

- `Abaqus::BlockNode`: Represents a keyword block (e.g., `*Part`, `*Assembly`, `*Step`). Holds a header map and nested children.
- `Abaqus::OptionNode`: Represents an option under a block (e.g., `*Node`, `*Element`, `*Nset`). Holds a header map and tokenized data rows.
- `Abaqus::HeaderMap`: Case-insensitive, presence-aware key-value map. Presence-only flags (e.g., `field`) return `true` when queried as `bool`.

## Parsing Rules

- Case-insensitivity: Keyword names and header keys are matched case-insensitively; values are preserved (trimmed).
- Continuations: A data line ending with a trailing comma joins with the next physical line before tokenization.
- Comments: Lines starting with `**` are ignored.
- Unknown keywords: Raise an error identifying the offending keyword.

## Responsibilities

- Input to output:
  - Input: stream of `.inp` lines.
  - Output: `BlockNode` root containing nested `BlockNode`/`OptionNode` children.
- No semantic guessing: The parser does not interpret node ids, instance scoping, UEL definitions, or set membership.

## Developer Notes

- Keep the parser “dumb”: Do not embed model-level assumptions here. Semantics belong in `AbaqusInputObjects`.
- HeaderMap conversions: Use `map.get<T>(key, default)` to retrieve values with proper types and defaults.

