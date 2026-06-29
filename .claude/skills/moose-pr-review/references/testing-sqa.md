# MOOSE Testing and SQA - Review Checklist

In MOOSE, **every test is a requirement** (`what_is_a_requirement.md`). Tests are not optional
extras - they are how the Software Requirement Specification stays correct, complete, and
traceable. A PR that adds behavior without a corresponding, properly specified test is
incomplete by definition. This is usually the area with the most required findings, so check it
carefully.

## Where tests live and the spec format

Tests are described in files literally named `tests` (HIT/getpot format) inside
`test/tests/<area>/` (framework) or `modules/<mod>/test/tests/<area>/`. A `tests` file contains
a `[Tests]` block with one or more test entries. Diff-based tests reference an input file
(`*.i`) and a "gold" reference output stored in a `gold/` subdirectory.

## The three SQA-required fields

Every test must be traceable. These three carry the traceability and are checked by the SQA
tooling:

- `requirement` - one **unambiguous** sentence stating what the software shall do. Concise; if
  it runs longer than a sentence, move detail into the `design` page. The requirement plus the
  test name should fully convey what is being verified.
- `design` - space-separated path(s) to the markdown design/system page(s), e.g.
  `'syntax/Positions/index.md'` or `'MyObject.md'`. Paths may be partial but must be unique.
- `issues` - space-separated issue numbers, e.g. `'#23587 #30899'`.

`design` and `issues` are commonly set once at the top of the `[Tests]` block and inherited (or
overridden) by each test. `requirement` is per test. When a single requirement is verified by
several related inputs, use sub-blocks with a shared `requirement` and a `detail = '...'` per
sub-block; the details read as a continuation of the requirement sentence.

## Annotated example (real structure)

```
[Tests]
  issues = '#23587 #30899 #31026'          # applies to all tests below unless overridden
  design = 'syntax/Positions/index.md'      # the design page these tests trace to

  [test]
    requirement = "The system shall be able to load positions from"   # completed by each detail
    [input]
      type = 'JSONDiff'                      # test type (see below)
      input = 'input_positions.i'            # the input file that drives the test
      jsondiff = 'input_positions_out.json'  # expected output compared against gold/
      detail = 'a parameter in the input file,'
    []
    [file]
      type = 'JSONDiff'
      input = 'file_positions.i'
      jsondiff = 'file_positions_out.json'
      detail = 'a text file,'
    []
  []
[]
```

## Common test types (pick the right one for what's being verified)

- `RunApp` - the input runs to completion (smoke/execution test); can check expected console
  output with `expect_out`.
- `Exodiff` - compares Exodus mesh/field output to a gold file (the workhorse for physics).
- `CSVDiff` - compares CSV postprocessor/reporter output to gold.
- `JSONDiff` - compares JSON output to gold.
- `RunException` - asserts the app errors as intended; pair with `expect_err = '...'`. **Every
  user-facing error path (`mooseError`/`paramError`) should have one of these.**
- `PythonUnitTest` - runs a Python `unittest` (for Python contributions and tooling).

## What to verify in review

- **Coverage of the change.** Does each new feature, parameter, and behavior have at least one
  test? Are the meaningful error conditions covered by `RunException`? CI checks a minimum
  coverage number; a reviewer may waive it for genuinely unreachable branches, but should say so.
- **The three fields are present and good.** Missing `requirement`/`design`/`issues` is a
  required finding. Is the `requirement` actually unambiguous, or vague ("works correctly")?
- **Gold files exist** for diff-based tests and were added in the PR (not stale/missing).
- **Right test type** for what's being checked (e.g. don't use `RunApp` where the numeric
  result is the point - use `Exodiff`/`CSVDiff`).
- **Test design quality.** Tests should be minimal and targeted, mirroring the
  one-test-one-requirement philosophy, rather than one giant input exercising everything.
- **Extra recipes when warranted.** Manual memory management -> consider suggesting a valgrind
  recipe; loose solver tolerances -> likely to fail parallel/recover testing.

Reference docs if you need depth: `python/doc/content/python/TestHarness.md` and
`modules/doc/content/application_development/test_system.md`.
