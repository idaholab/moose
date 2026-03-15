# TaoGradientTester

The `TaoGradientTester` tester uses TAO's gradient testing functionality to compare
hand-coded (adjoint) gradients against finite-difference gradients. It automatically
injects the necessary PETSc options to perform the gradient test and parses the output
to verify the gradient is accurate within specified tolerances.

This tester is useful for verifying that adjoint-based gradient computations are
correct in optimization problems. See the [debugging help](optimization/examples/debuggingHelp.md)
page for more information on debugging optimization gradients.

## Options

Test configuration options are specified in the `tests` file.

- `cosine_tol`: Tolerance for `|1 - angle_cosine|`. The angle cosine between the
  hand-coded and finite-difference gradients should be 1.0 for a perfect gradient.
  Defaults to 1e-2.

- `max_norm_tol`: Tolerance for the absolute max-norm `||G - Gfd||`. Should be near
  zero for a correct gradient. Defaults to 1e-6.

- `tao_solver`: TAO solver to use for gradient testing. Defaults to `taobncg`.

- `tao_fd_delta`: Finite difference step size (`-tao_fd_delta`) used by TAO when
  computing the finite-difference gradient. This is problem dependent. If not
  specified, TAO uses its default.

The tester automatically sets the following:

- `recover = false` and `restep = false`
- Valgrind is disabled
- Requires `method=opt` (optimized build)

Other test commands & restrictions may be found in the MOOSE TestHarness documentation.

## How It Works

The tester automates the manual gradient testing setup that would otherwise need to be
added directly to the `Executioner` block of an input file, such as:

!listing test/tests/executioners/basic_optimize/debug_gradient.i block=Executioner

Output from the gradient test in the Executioner block will look like this when the gradient is correct:

!listing executioners/basic_optimize/gold/debug_gradient.out start=||Gfd|| end=TAO SOLVER include-end=True

The tester prepends CLI arguments to override the input file's `Executioner` block,
configuring TAO to:

1. Run only a single optimization iteration (`-tao_max_it 1`)
2. Enable finite-difference gradient testing (`-tao_fd_test`, `-tao_test_gradient`)
3. Use unit line search to minimize extra solves (`-tao_ls_type unit`)
4. Print the gradient comparison (`-tao_test_gradient_view`)

After running, the tester parses two values from the TAO output:

- +Angle cosine+: Computed as `(Gfd'G) / (||Gfd|| ||G||)`, this is the cosine of the
  angle between the hand-coded gradient `G` and the finite-difference gradient `Gfd`.
  A value close to 1 means the two gradient vectors are pointing in the same direction,
  confirming the adjoint gradient has the correct direction. The test checks that
  `|1 - angle_cosine| < cosine_tol`.
- +Max-norm+: `||G - Gfd||`, the max-norm of the difference between the hand-coded and
  finite-difference gradients. Should be near zero for a correct gradient, confirming
  the adjoint gradient also has the correct magnitude. The test checks that this value
  is less than `max_norm_tol`.

## Example test configuration in the MOOSE test suite

In this example, `TaoGradientTester` is used to verify the adjoint gradient for a
point load inversion problem:

!listing modules/optimization/test/tests/optimizationreporter/point_loads/tests block=point_loads/gradient_test
