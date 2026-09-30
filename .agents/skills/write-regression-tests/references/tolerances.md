# Diff tester tolerances and comparison files

How `Exodiff`, `CSVDiff`, and `JSONDiff` size their comparisons, and how to give one column or
variable its own tolerance. This is the single source for those mechanics; the skills that need
them cite this file. `brittle-numerics-root-cause` owns the separate judgment call of whether
widening a tolerance is the right fix at all.

## The spec-wide defaults

`FileTester.validParams()` gives every diff tester `rel_err = 5.5e-6`, `abs_zero = 1e-10`, and
`gold_dir = 'gold'`. Both tolerances are always passed explicitly on the comparison tool's command
line, so the spec values are what the tool sees:

- `Exodiff` invokes `exodiff -F <abs_zero> -t <rel_err>`, plus `-m` from `map` and anything in
  `exodiff_opts`.
- `CSVDiff` passes the same two values to `csvdiff.py`. That script's own argparse defaults match,
  and apply only when it runs standalone outside the TestHarness.

## `CSVDiff`

`CSVDiff` offers two ways to give a column its own tolerance.

`override_columns`/`override_rel_err`/`override_abs_zero` map to `csvdiff.py`'s
`--custom-columns`/`--custom-rel-err`/`--custom-abs-zero` and give named columns their own
tolerance while the rest of the file stays at the spec values. Prefer these for a column or two.
All three lists must be the same length, or `CSVDiff.checkRunnable` fails the test with "Override
inputs not the same length". Widening `rel_err` for one column therefore also requires that
column's `override_abs_zero`.

`comparison_file` names a file, resolved relative to the test directory, that the tester hands to
`csvdiff.py --comparison-file`. It uses the exodiff command-file syntax: an unindented header line
carrying `relative`/`floor`, then one indented line per column, optionally with its own
`relative`/`floor`.

- The file restricts the diff to the columns it lists, so every unlisted column goes unchecked.
  Generate it with `csvdiff.py --summary gold/<file>.csv`, which lists every column, and then edit
  only the affected column's line.
- The header's `relative`/`floor` replace the spec's `rel_err`/`abs_zero`. Without them, the listed
  columns are compared with a relative tolerance and floor of zero, which demands an exact match.
- `csvdiff.py` rejects a comparison file combined with the `override_*` parameters.

With either mechanism, `csvdiff.py` reports the first mismatching row per column and then stops, so
the number it prints is the first offender rather than the worst-case relative diff over the run.
Size a tolerance by replaying its abs_zero/rel_diff logic over the full column.

## `Exodiff`

`Exodiff` has no `override_columns` equivalent. `custom_cmp` instead names a command file, resolved
relative to the test directory, that the tester hands to `exodiff -f`. Generate that file with
`exodiff -summary` and edit the result:

- `exodiff -summary gold/<file>.e` prints every variable already grouped into `GLOBAL VARIABLES`,
  `NODAL VARIABLES`, and `ELEMENT VARIABLES` blocks, each line carrying a `# min: ... max: ...`
  comment with that variable's file-wide peak. Keep those comments: they are tool-sourced evidence
  for whatever tolerance the file ends up asserting.
- Give every block header `(all)` with no `relative` or `floor` of its own. `(all)` then falls
  through to the `-F <abs_zero> -t <rel_err>` the tester passes from the spec, so the rest of the
  file stays exactly as strict as it was and stays correct if the spec tolerances change later.
  `(all)` also keeps the file working as new output variables appear.
- exodiff's `Parse_Variables` parses a block's `(all)` flag and its
  indented per-variable override list independently of each other and of the block type. A block
  holding one affected variable therefore needs only that variable's override line under `(all)`,
  and every unaffected variable can stay out of the file.
- Put `relative` or `floor` only on the affected variable's line, with a comment giving the reason.

Look for a sibling `.cmp` file in the test suite before inventing a format: a neighboring test that
already floors a near-zero variable is a precedent to extend.
