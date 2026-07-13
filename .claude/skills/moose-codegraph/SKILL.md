---
name: moose-codegraph
description: >-
  Use CodeGraph to navigate MOOSE and avoid reinventing functionality it already has (MOOSE's
  "use existing functionality, don't reimplement it" rule - AGENTS.md, Simplicity First). Use
  this whenever writing or changing MOOSE C++/Python - before adding a new function, class,
  material, kernel, action, or utility - to check whether MOOSE, libMesh, MetaPhysicL, or
  MathUtils already provides it; to find the call sites and blast radius of a change; or to
  compare a new object against an existing sibling. CodeGraph is preferred but optional here: if
  the index is absent, fall back to grep/Read and suggest setting it up rather than blocking.
---

# CodeGraph for MOOSE

CodeGraph is a pre-built knowledge graph of every symbol, edge, and file in MOOSE. It answers
"does this already exist?" and "what does this touch?" in one sub-second call - exactly what you
need to honor MOOSE's Simplicity-First rule ("if existing functionality can accomplish a subtask,
use it, don't reimplement it; if it's nearly capable, prefer a short extension over a long
rewrite") and something grep cannot do reliably at this scale.

Prefer the MCP tools (`codegraph_explore`, `codegraph_search`, `codegraph_node`,
`codegraph_callers`). If they aren't loaded, the shell equivalents print the same output:
`codegraph explore "..."`, `codegraph node <name>`, `codegraph query <name>`,
`codegraph callers <name>`, `codegraph impact <name>`.

## Availability (optional - do not block on it)

Confirm `.codegraph/` exists at the repo root, or run `codegraph status` (it prints the file/
node/edge counts). If it is missing, CodeGraph is not set up for this repo:

- Fall back to grep/Read for the same questions - do not stop, and
- Suggest setting it up once so future sessions are faster and more reliable:

```bash
codegraph install   # one-time: register the CodeGraph MCP server with this agent
codegraph init      # build the index for this repo (run from the repo root; takes a few minutes)
```

The index lags writes by ~1s via a file watcher, so it reflects the working tree.

(The `moose-pr-review` skill *requires* CodeGraph and stops without it. General coding does not -
this is the soft entry point.)

## Primary use: don't reinvent existing functionality

Before you add a new function, class, or helper, check whether MOOSE already provides it.

**1. Name the symbol you are about to write** - the function/class/method and what it does.

**2. Ask whether MOOSE already has it**, from two angles, because a reinvented helper is often
named differently than the original:

- By name/near-name: `codegraph_search` the name and obvious variants (a new `computeNormalVector`
  -> search `normal`, `normalize`, `unitVector`).
- By concept: `codegraph_explore` a short description of what the code does ("interpolate value at
  quadrature point", "parse comma separated list to vector", "L2 norm of a vector"). This returns
  the verbatim source of the closest existing symbols, so you can compare behavior directly.

**3. Act on the finding.** If an existing symbol does the same thing, reuse it - or extend/fix it
if it's nearly capable (a short extension is preferred over a long reimplementation). Only write
new code if the existing one genuinely differs (different semantics, performance contract, or it
is deprecated).

Common MOOSE reinvention hot-spots: math/tensor helpers (often already in `libMesh`,
`MetaPhysicL`, or `MathUtils`), string/parsing utilities (`MooseUtils`), mesh/geometry queries,
and "generic" materials/functors that mirror an existing templated one.

Note: the default `codegraph.json` excludes the `libmesh` and `contrib` trees (so `MetaPhysicL`
too) to keep the index small and fast, so CodeGraph will not surface a helper reinvented from
those. For the libMesh/MetaPhysicL hot-spots, fall back to grep or prior knowledge - or index
those trees explicitly when the check matters for a given PR.

## Other uses

- **Blast radius before you change a shared symbol.** `codegraph_callers` / `codegraph impact`
  lists every call site, so you can update them all and gauge how risky the change is.
- **Follow the established pattern.** Before writing a new object, `codegraph_explore` a sibling
  that already does something similar (a new `Kernel` -> an existing comparable `Kernel`) and
  match its structure unless there's a reason not to.
- **Verify a claim.** When a comment or PR says "this mirrors X" or "replaces Y", pull X/Y with
  `codegraph_node` (`includeCode=true`) and confirm it instead of taking it on faith.

## Practical notes

- `codegraph_explore` is the workhorse - one call returns relevant symbols' source grouped by
  file plus the call paths between them, so it usually answers the question without follow-up
  Reads. Treat its output as already-Read.
- For an ambiguous name (many overloads/same-named classes across modules), `codegraph_node`
  returns every matching definition's body; narrow with the `file` argument.
- Keep queries specific. Bag-of-symbols ("MathUtils pow raisePower") and natural-language
  questions both work.
