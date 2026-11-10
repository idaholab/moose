# Jacobian tests

`PetscJacobianTester` runs the input with `-snes_test_jacobian` and compares the hand-coded
Jacobian `J` against PETSc's finite-difference Jacobian `Jfd`. AGENTS.md requires one for every
added or changed hand-coded Jacobian; this file covers what that test has to exercise and how to
read its result.

## What the test must exercise

A Jacobian test only checks the entries the input actually assembles and couples, so the input,
not the tester, decides what is verified.

- Couple everything the residual depends on. An off-diagonal block is compared only when the
  coupled variable is a nonlinear variable in the same system and the preconditioner assembles that
  block: use `[Preconditioning]` with `SMP` and `full = true`, or list the coupling with
  `off_diag_row`/`off_diag_column`. A coupled variable left as an `AuxVariable` hides its
  off-diagonal block from the comparison entirely.
- Start from a nontrivial state. Initialize every variable with a non-constant, non-linear
  `FunctionIC` so that no derivative term vanishes by accident; a zero gradient or a constant field
  makes a missing term invisible.
- Cover each coordinate system and geometry branch the change touches, e.g. Cartesian and `RZ`
  through `cli_args = 'Mesh/coord_type=RZ'` on the same input, per the coverage rule in AGENTS.md.
- Keep the mesh small, a few elements per direction. Finite differencing costs one residual
  evaluation per column.

## Tolerances

The tester passes when `||J - Jfd||_F / ||J||_F < ratio_tol` (default `1e-7`). It also checks
the absolute difference `||J - Jfd||_F < difference_tol` when a spec sets `difference_tol`; that
check is disabled by default.

- The ratio is the meaningful check. A correct Jacobian typically reports a ratio of `1e-8` to
  `1e-9`, the size of the finite-difference error, and a missing or wrong term reports a ratio well
  above `1e-7`, often `1e-3` or larger.
- The absolute difference grows with the magnitude of the Jacobian entries: material coefficients,
  field magnitudes, mesh size, and units. Set `difference_tol` only with a reason recorded next to
  it in the spec, sized for that problem; a value copied from a test of different scale either
  fails a correct Jacobian or checks nothing.

## Telling a wrong Jacobian from noise

When a test fails, establish which it is before changing a tolerance or the code.

- Rerun the input directly with a few finite-difference step sizes, e.g. `-snes_test_err 1e-8`,
  `1e-6`, and `1e-4`. For a correct Jacobian the ratio is pure finite-difference error and changes
  with the step; for a wrong Jacobian it stays at the size of the missing or wrong term whatever
  the step.
- Run a control that is expected to fail, such as dropping the off-diagonal blocks with
  `Preconditioning/smp/full=false`. A control ratio of order `1e-1` confirms the test is sensitive
  to the terms it claims to verify.
- To localize a wrong entry, run the input directly with `-snes_test_jacobian_view`, which prints
  `J`, `Jfd`, and their difference. A debug build is useful at this point for stepping into the
  kernel that produced the entry.

## Running it

The tester adds `method=opt` to the test's capabilities, so a `METHOD=dbg` run reports the test as
skipped with `[Need ... method=opt]`. Pass `--ignore-capability=method` to `run_tests` to run it
against a debug build.
