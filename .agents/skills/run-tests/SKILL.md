---
name: run-tests
description: >-
  Use when running or debugging MOOSE regression tests (run_tests / TestHarness) in the MOOSE
  framework itself or any MOOSE-based application (e.g. bison and other downstream apps). Covers
  locating run_tests, scoping to a subset of tests, and interpreting failures.
---

# MOOSE Regression Tests (run_tests)

## Where it lives
- App repos (bison, etc.): `./run_tests` at the repo root. It wraps MOOSE's TestHarness, locating
  MOOSE via the `MOOSE_DIR` env var or a `moose/` submodule.
- MOOSE repo itself has no top-level `run_tests` — it's per test-group instead:
  `moose/test/run_tests` (framework tests), `moose/modules/run_tests` (all modules),
  `moose/modules/<module>/run_tests` (one module), `moose/python/run_tests`, `moose/unit/run_tests`.
  They're all the same thin TestHarness wrapper, just rooted at different directories.

Run from the directory the script lives in (repo root for an app; `test/`, `modules/`,
`modules/<module>/`, etc. for MOOSE itself). The app/module must already be compiled (opt/dbg
binary present, e.g. after `make -j<N>`).

## Basic usage
```bash
./run_tests -j<N>              # full suite, N parallel jobs
./run_tests -j<N> --re <regex> # only tests whose name/path matches <regex>
```
`run_tests` picks the binary matching `$METHOD` (default `opt`, e.g. `bison-opt`). Set
`METHOD=dbg` (or `oprof`/`devel`) to test a different build — it must be compiled with that same
`METHOD` first (`make METHOD=dbg -j<N>`).
To scope to one directory, `cd` into it and invoke run_tests via a relative path — TestHarness
only collects tests at/below the current working directory:
```bash
cd test/tests/<some>/<dir> && ../../../../run_tests -j<N>
```

## Stable flags
Flag names/behavior can drift slightly across MOOSE versions, but these, along with aforementioned `-j` and `--re` are stable:
- `--heavy`: run tests marked with `heavy` (skipped by default)
- `-v`, `--verbose`: show the output of every test
- `--recover`: run tests in recover mode
- `-p <N>`, `--parallel <N>`: number of MPI processes to use for each job
- `--distributed-mesh`: run tests that support distributed mesh (passes `--distributed-mesh` to
  the executable)
- `--n-threads <N>`: number of threads to use when running mpiexec

For anything else, run `./run_tests -h` in that repo and confirm against its actual output rather
than assuming.

Each test consumes `-p` x `--n-threads` slots, and `-j` is a hard cap on total slots in use at
once. If `-j` is less than a test's `-p` x `--n-threads`, that test is skipped with an
"insufficient slots" caveat instead of running.

## Reading failures
- Output names the test and the failure reason: DIFF, ERROR, CRASH, TIMEOUT, EXODIFF, CSVDIFF...
- For a numeric diff (EXODIFF/CSVDIFF), rerun just that test with `--verbose` for details, and
  check the test's spec file (named `tests`) for the `exodiff`/`csvdiff`/`abs_zero`/`rel_err`
  parameters controlling the comparison.
- Gold files live in `gold/` next to the input. Don't regold or relax test tolerances or parameters without understanding *why* the
  result changed — if the diff only appears under certain compiler flags/hardware/thread counts,
  use the brittle-numerics-root-cause skill instead of just re-golding.
- Re-golding or relaxing tolerances or parameters is always a last resort measure, only to be used after confirming that any code changes leading to the diff undoubtedly improve the code's scientific correctness or accuracy.
- Changing a tolerance is a test-authoring decision and lives in other skills:
  brittle-numerics-root-cause decides whether widening one is the right fix at all, and
  `.agents/skills/write-regression-tests/references/tolerances.md` holds the mechanics for
  `CSVDiff`'s `override_columns` and `Exodiff`'s `custom_cmp`.
- For a CRASH/ERROR, rerun the input directly with the app binary (e.g. `./bison-opt -i
  <input>.i`) to get a full stack trace instead of TestHarness's summarized output.

## Cross-repo notes
The script and flags work the same whether you're in `moose/`, a module under
`moose/modules/`, or a downstream app — only the aggregation scope (which tests are visible)
changes. If a downstream app's tests need an updated MOOSE, that's a submodule-pointer issue, not
a run_tests issue.
