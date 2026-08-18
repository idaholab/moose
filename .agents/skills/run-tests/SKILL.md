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

## Always confirm flags before relying on them
Flag names/behavior drift slightly across MOOSE versions. `-j` and `--re` above are stable; for
anything else (`--heavy`, `-i` for an alternate spec filename, `--verbose`, `--update-gold`,
`--recover`, etc.) run `./run_tests -h` in that repo and confirm against its actual output rather
than assuming.

## Reading failures
- Output names the test and the failure reason: DIFF, ERROR, CRASH, TIMEOUT, EXODIFF, CSVDIFF...
- For a numeric diff (EXODIFF/CSVDIFF), rerun just that test with `--verbose` for details, and
  check the test's spec file (named `tests`) for the `exodiff`/`csvdiff`/`abs_zero`/`rel_err`
  parameters controlling the comparison.
- Gold files live in `gold/` next to the input. Don't regold without understanding *why* the
  result changed — if the diff only appears under certain compiler flags/hardware/thread counts,
  use the brittle-numerics-root-cause skill instead of just re-goldening.
- For a CRASH/ERROR, rerun the input directly with the app binary (e.g. `./bison-opt -i
  <input>.i`) to get a full stack trace instead of TestHarness's summarized output.

## CSVDiff tolerance overrides
- `CSVDiff` tests support `override_columns`/`override_rel_err`/`override_abs_zero` (mapped to
  `csvdiff.py`'s `--custom-columns`/`--custom-rel-err`/`--custom-abs-zero`) to give specific
  columns a different tolerance than the test's own `rel_err`/`abs_zero`. Prefer this over
  loosening the whole test's `rel_err` when only one or two columns need slack — e.g. a quantity
  computed as the residual of much larger, comparable-magnitude terms that happens to be near a
  physical zero (see brittle-numerics-root-cause's note on this).
- All three lists must be the same length or the test fails with "Override inputs not the same
  length" (`CSVDiff.py`'s `checkRunnable`) — even if you only want to widen `rel_err` for a column,
  you must still supply a matching `override_abs_zero` entry for it.
- The effective default `rel_err`/`abs_zero` for *non-overridden* columns comes from the Tester's
  own `FileTester.validParams()` defaults (`rel_err=5.5e-6`, `abs_zero=1e-10`), which are always
  passed explicitly to `csvdiff.py` (these match `csvdiff.py`'s own argparse defaults, which only
  matter when it's run standalone outside the TestHarness).
- `csvdiff.py` reports only the *first* mismatching row per column, then stops — it does not report
  the true worst-case relative diff over the run. To size a tolerance correctly, replicate its
  abs_zero/rel_diff logic over the full column yourself rather than trusting the reported value as
  the max.

## Cross-repo notes
The script and flags work the same whether you're in `moose/`, a module under
`moose/modules/`, or a downstream app — only the aggregation scope (which tests are visible)
changes. If a downstream app's tests need an updated MOOSE, that's a submodule-pointer issue, not
a run_tests issue.
