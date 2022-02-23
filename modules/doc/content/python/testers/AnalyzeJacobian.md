# AnalyzeJacobian

The `AnalyzeJacobian` tester uses the
[analyze_jacobian.py Jacobian debugger](help/development/analyze_jacobian.md)
in MOOSE to evaluate the quality of the Jacobians in the specified tests.

## Options

Test configuration options are added to the `tests` file.

- `expect_out`: a regular expression that must occur in the input in order for the test to be considered passing.

- `resize_mesh`: whether to resize the input mesh, defaults to `False`

- `off_diagonal`: whether to also test the off-diagonal Jacobian entries, defaults to `True`

- `mesh_size`: degree of refinement of the mesh size, to set if `resize_mesh` is `True`


Other test commands & restrictions may be found in the [TestHarness documentation](TestHarness.md).

## Example test configuration in the MOOSE test suite

In this example, three `AnalyzeJacobian` tests are created to check the Jacobian created by a finite difference
preconditioner. The success of the Jacobian analysis is checked by looking for the string "No errors detected"
in the output.

!listing test/tests/preconditioners/fdp/tests
