# CodeGraph for MOOSE Review

CodeGraph is a pre-built knowledge graph of every symbol, edge, and file in MOOSE. It answers
"does this already exist?" and "what does this touch?" in one sub-second call, which is exactly
what a reviewer needs and what grep cannot do reliably at this scale. Using it well is the
difference between catching a reinvented utility and merging it.

Prefer the MCP tools. If they aren't loaded, the shell equivalents print the same output:
`codegraph explore "..."`, `codegraph node <name>`, `codegraph query <name>`,
`codegraph callers <name>`, `codegraph impact <name>`.

## Preflight (hard gate)

- Confirm `.codegraph/` exists at the repo root, or run `codegraph status` (it prints the file/
  node/edge counts). If absent, stop and tell the user to run `codegraph install` then
  `codegraph init` - the review cannot do its most important job without the index.
- The index lags writes by ~1s via a file watcher, so it reflects the working tree. If the PR
  branch was just checked out, that's fine; the graph indexes current files.

## Primary job: catch reinvented functionality

MOOSE's Simplicity-First rule is "if existing functionality can accomplish a subtask, use it,
don't reimplement it; if it's nearly capable, prefer a short extension over a long rewrite."
CodeGraph is how you verify a PR honors this.

**1. Extract the new symbols from the diff.** Functions, classes, methods, and free helpers
that the PR adds (added lines defining `class X`, `Type X::method(...)`, `registerMooseObject`,
anonymous-namespace helpers in `.C` files).

**2. For each new symbol, ask whether MOOSE already has it.** Use two complementary angles -
name and concept - because a reinvented helper is often named differently than the original:

- By name/near-name: `codegraph_search` with the symbol name and obvious variants
  (e.g. a new `computeNormalVector` -> search `normal`, `normalize`, `unitVector`).
- By concept: `codegraph_explore` with a short description of what the code does
  (e.g. "interpolate value at quadrature point", "parse comma separated list to vector",
  "L2 norm of a vector"). This returns the verbatim source of the closest existing symbols, so
  you can compare behavior directly.

**3. Judge the finding.** If an existing symbol does the same thing:
- Required: reuse it, or extend/fix it if it's nearly capable. Point the author to the exact
  `file:line` (CodeGraph returns it) so the suggestion is actionable.
- If the new code legitimately differs (different semantics, performance contract, or the
  existing one is deprecated), say why reuse doesn't apply - don't flag reflexively.

Common MOOSE reinvention hot-spots worth a targeted look: math/tensor helpers (often already in
`libMesh`, `MetaPhysicL`, or `MathUtils`), string/parsing utilities (`MooseUtils`), mesh/geometry
queries, and "generic" materials/functors that mirror an existing templated one.

## Secondary jobs

**Blast radius / scope.** For an existing symbol the PR modifies, run `codegraph_callers` (or
`codegraph impact`) to see how widely it's used. A change to a heavily-called API deserves
greater scrutiny (matches `reviewing.md`: scope-proportional review) and the PR should update
all affected call sites - CodeGraph lists them, so check none were missed.

**Consistency with existing patterns.** Before judging a new object's design, `codegraph_explore`
a sibling that already does something similar (e.g. reviewing a new `Kernel` -> explore an
existing comparable `Kernel`). New code should follow the established pattern unless there's a
reason not to.

**Verifying author claims.** When a PR description or comment says "this mirrors X" or "replaces
Y", pull X/Y with `codegraph_node` (use `includeCode=true`) and confirm the claim instead of
taking it on faith.

## Practical notes

- `codegraph_explore` is the workhorse - one call returns relevant symbols' source grouped by
  file plus the call paths between them, so it usually answers the question without follow-up
  Reads. Treat its output as already-Read.
- For an ambiguous name (many overloads/same-named classes across modules), `codegraph_node`
  returns every matching definition's body; narrow with the `file` argument.
- Keep queries specific. Bag-of-symbols ("MathUtils pow raisePower") and natural-language
  questions both work.
