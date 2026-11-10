# FileTester

Base class for all the testers comparing output files to reference gold files.
It handles the selection of the `gold` folder, containing the reference files,
and the value of the absolute and relative tolerances.

## Common parameters

- `gold_dir`: The directory, relative to the test directory, that holds the reference files. Defaults to `./gold/`.

- `abs_zero`: Absolute zero cutoff, defaults to `1e-10`.

- `rel_err`: Relative error tolerance, defaults to `5.5e-6`.

Each tester turns these parameters into a pass/fail decision in its own way. In the equations
on the tester pages, $a$ is the value in the reference (gold) file, $b$ is the value in the
test output, $z$ is `abs_zero`, and $\tau_\mathrm{rel}$ is `rel_err`.

| Tester | Values compared | Role of `abs_zero` | Role of `rel_err` |
| :- | :- | :- | :- |
| [Exodiff](Exodiff.md#comparison) | Every nodal, element, and global variable at every time step | Floor: a pair is skipped if both values are within it | Relative tolerance, normalized by the larger magnitude |
| [CSVDiff](CSVDiff_tester.md#comparison) | Every value in every column | Floor, as in `Exodiff` | Relative tolerance, normalized by the larger magnitude; `abs_err` adds an absolute tolerance |
| [JSONDiff](JSONDiff.md#comparison), [XMLDiff](XMLDiff.md#comparison) | Every numeric value | Floor: a pair is skipped if both values are below it | Relative tolerance, normalized by the test value |
| [ImageDiff](ImageDiff.md#comparison) | The whole image | Not used | Not used; `allowed` bounds the structural similarity |

The tolerances of [CSVValidationTester](CSVValidationTester.md#comparison) are statistics of the
error over all values and do not use these parameters.
