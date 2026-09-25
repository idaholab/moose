---
name: chunked-file-writer
description: Write source files incrementally in small chunks instead of one large Write. Use whenever creating or generating any file longer than ~80 lines (scripts, modules, reports, configs), or when the user asks to "write the script/file/module". Prevents API stream timeouts on this network by keeping every single tool call small.
---

# Chunked File Writer

This environment routes API traffic through a gateway that drops long streaming
responses. Any single tool call whose generated content exceeds roughly 100
lines is likely to stall the session. Therefore, NEVER write a large file in
one Write call. Follow this procedure for every file longer than ~80 lines.

## Procedure

1. **Plan the sections first.** Before writing anything, list the file's
   sections (e.g., header/docstring, constants, function 1, function 2, ...,
   main, entry point). Aim for sections of 40-80 lines each. State the section
   list in one short sentence, then begin.

2. **Write the skeleton.** Create the file with a single small Write
   containing ONLY:
   - the header comment/docstring,
   - imports and constants,
   - a one-line placeholder comment for each planned section, in order, e.g.:
     `# SECTION 2: reactor_period_regression - implemented in a later edit`
   The skeleton must be under 60 lines.

3. **Fill sections one at a time with Edit.** For each section, make ONE Edit
   call that replaces that section's placeholder comment with the full
   implementation. Hard limits per Edit:
   - max ~80 lines / ~3000 characters of new content;
   - if a section is bigger than that, split it into part A / part B with its
     own placeholder for part B.
   Never combine two sections into one Edit. Never rewrite the whole file.

4. **No batching, no parallel writes.** One section per tool call, each in its
   own turn of tool use. Keep any prose between tool calls to a single short
   sentence.

5. **Verify at the end.** After the last section:
   - run a syntax check (e.g., `python -m py_compile <file>` for Python);
   - grep the file for remaining `SECTION` placeholders to confirm none are
     left: `grep -n "SECTION" <file>` should return nothing (or only
     intentional matches);
   - report the final line count.

## Also applies to

- Rewrites and refactors: modify existing large files with several small
  targeted Edits, never one whole-file replacement.
- Long markdown/reports/configs: same skeleton-then-sections procedure.
- Small files (< 80 lines): write normally in a single call; this procedure
  is unnecessary overhead there.

## If a stall happens anyway

If a tool call appears to hang or a turn is interrupted, resume by checking
which placeholders remain in the file and continue from the first unfilled
section. Never restart the file from scratch.
