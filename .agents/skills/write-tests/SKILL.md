---
name: write-tests
description: >-
  Use when adding or writing a MOOSE regression test - a new `tests` spec entry, its input file,
  or its gold file - in the MOOSE framework or any MOOSE-based application. Covers where the
  files go, picking the tester type, creating the gold file, and the spec parameters that express
  parallel/threaded/recover coverage. Use `run-tests` to run what you wrote and
  `pr-review/references/testing-sqa.md` for the requirement/design/issues traceability rules.
---

# Writing MOOSE tests

In MOOSE every test is a requirement, so a new feature, parameter, or error path is incomplete
until a test specifies it. Read
`.agents/skills/pr-review/references/testing-sqa.md` before writing the spec: it holds the
`requirement`/`design`/`issues` rules, how those fields are inherited from the top of the
`[Tests]` block, and the `detail =` sub-block pattern for one requirement verified by several
inputs. That reference is the single source for those rules.

## Where the files go

One directory per case, holding everything the case needs:

```
test/tests/<area>/<case>/tests                 # spec file, literally named "tests" (HIT format)
test/tests/<area>/<case>/<case>.i              # the input that drives it
test/tests/<area>/<case>/gold/<case>_out.e     # the reference output
```

Module tests live under `modules/<module>/test/tests/<area>/<case>/` with the same shape. Add to
an existing `tests` file when the new test belongs to the same area; the SQA fields at the top of
its `[Tests]` block then already cover it.

## Picking the tester type

`type` selects a tester from `python/TestHarness/testers/`:

| type | verifies |
| - | - |
| `Exodiff` | Exodus mesh/field output against gold |
| `CSVDiff` | CSV postprocessor/reporter output against gold |
| `JSONDiff` | JSON (reporter) output against gold |
| `RunException` | the app errors as intended; pair with `expect_err = '...'` |
| `RunApp` | the input runs at all; `expect_out` to check console text |
| `CheckFiles` | named files are produced (`check_files`), without diffing content |
| `PythonUnitTest` | a Python `unittest` module |

Others exist for narrower jobs (`ImageDiff`, `SchemaDiff`, `XMLDiff`, `AnalyzeJacobian`,
`PetscJacobianTester`, `MMSTest`).

Choose by what actually needs to hold:

- When a number is the point, use a diff test. `RunApp` passes while the answer is wrong, so it
  earns its place only as a pure "does it run" smoke check.
- Every user-facing `mooseError`/`paramError` deserves a `RunException` with `expect_err` matching
  a stable fragment of the message, or a C++ unit test.
- `AnalyzeJacobian` or `PetscJacobianTester` for hand-coded Jacobians.

## Creating the gold file

There is no TestHarness flag that golds a test for you. A diff tester compares
`<test dir>/<file>` against `<test dir>/<gold_dir>/<file>` (`gold_dir` defaults to `gold`), so the
gold file carries the same basename as the value of `exodiff`/`csvdiff`/`jsondiff`:

```bash
cd test/tests/<area>/<case>
<path to the app binary> -i <case>.i   # writes <case>_out.e in this directory
mkdir -p gold && mv <case>_out.e gold/
```

Then run the test through `run_tests` to confirm it passes against its own gold.

- A missing gold file fails the test with `File Not Found`, naming the path it expected.
- `delete_output_before_running` defaults to true, so the harness removes the output file from the
  test directory before each run. The copy in `gold/` is the only one that persists.
- Keep gold files small: coarse mesh, few timesteps, and output limited to the quantities being
  diffed. They are committed, so a large gold file is a permanent cost.
- Inspect a new gold file before committing it. Golding a wrong answer specifies a wrong
  requirement, and the test then defends the bug.

## Coverage across configurations

Per AGENTS.md, one test plus TestHarness options covers process counts, thread counts, distributed
mesh, recover, and restep. Do not write sibling specs that differ only in those. Split a test only
when a mode intentionally has different inputs, expected output, or requirements.

State a test's real needs with spec parameters:

- `min_parallel`/`max_parallel`, `min_threads`/`max_threads` - the process and thread counts the
  test requires. Give the harness `-j` of at least `-p` x `--n-threads`, or the test is skipped
  with an "insufficient slots" caveat.
- `capabilities = '...'` - build-dependent tests, e.g. `'method!=dbg'`, `'superlu'`, `'kokkos'`,
  `'mfem'`, `'installation_type=in_tree'`. This replaces the older per-feature params
  (`method`, `petsc_version`, `superlu`, `installation_type`, ...), which are deprecated in favor
  of it (`Tester.capability_params`).
- `recover = false`, `restep = false` - only when the test genuinely cannot support that mode.
- `heavy = true` - long-running tests, skipped unless `--heavy`.
- `prereq = '<test name>'` - when a test consumes another's output.
- `valgrind = 'NONE|NORMAL|HEAVY'` - manual memory management is worth a valgrind recipe.

## Tolerances

`Exodiff`/`CSVDiff` default to `rel_err = 5.5e-6` and `abs_zero = 1e-10`
(`FileTester.validParams`). Set them as tight as the physics allows: a loose tolerance makes the
test unable to fail. When one column needs slack, `CSVDiff`'s `override_columns`/
`override_rel_err`/`override_abs_zero` beat widening the whole test, and `run-tests` documents
their length-matching requirement.

## Before calling the test done

- Run it with `run-tests`, then run it in the modes it claims to support.
- A new registered object also needs its documentation page, or the `design` field has nothing
  valid to point at. Use the `moose-docs` skill.
- Use `verify-changes` for the build-and-verify gate rather than asserting a test passes.
