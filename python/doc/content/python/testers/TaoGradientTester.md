# TaoGradientTester

The `TaoGradientTester` uses TAO's gradient testing functionality to compare
hand-coded (adjoint) gradients against finite-difference gradients. It automatically
injects the necessary PETSc options into the optimization Executioner (see [executionerBlock]) to perform the gradient test and parses the output
to verify the gradient is accurate within specified tolerances.

This tester is useful for verifying that adjoint-based gradient computations are
correct in optimization problems. See the [debugging help](optimization/examples/debuggingHelp.md)
page for more information on debugging optimization gradients.

## Options

Test configuration options are specified in the `tests` file, see [testBlock].

- `max_rel_tol`: Tolerance for the relative max-norm `||G - Gfd||/||G||`. Should be near
  zero for a correct gradient. Defaults to 1e-5.

- `tao_solver`: TAO solver to use for gradient testing. Defaults to `taobncg`.

- `tao_fd_delta`: Finite difference step size (`-tao_fd_delta`) used by TAO when
  computing the finite-difference gradient. This is problem dependent. If not
  specified, TAO uses its default.

- `only_first_gradient`: Check only the first gradient comparison. Defaults to True.
  It is best to check the first gradient since later gradients are evaluated near
  convergence where the gradient magnitude is small, making the relative finite-difference
  error larger. Set to False to check all gradient comparisons in the output.

The tester automatically sets the following:

- `recover = false` and `restep = false`
- Valgrind is disabled
- Requires `method=opt` (optimized build)

Other test commands & restrictions may be found in the MOOSE TestHarness documentation.

## How It Works

The tester automates the manual gradient testing setup that would otherwise need to be
added directly to the `Executioner` block of an input file, such as:

!listing modules/optimization/test/tests/executioners/basic_optimize/debug_gradient.i block=Executioner
         id=executionerBlock
         caption=Example Executioner block for manual gradient testing with TAO.

Output from the gradient test in the Executioner block will produce the following output comparing the finite difference gradient `Gfd` to the adjoint gradient computed by the code `G`:

!listing modules/optimization/test/tests/executioners/basic_optimize/gold/debug_gradient.out start=||Gfd|| end=TAO SOLVER include-end=True

The tester prepends CLI arguments to override the input file's `Executioner` block,
configuring TAO to:

1. Run only a single optimization iteration (`-tao_max_it 1`)
2. Enable finite-difference gradient testing (`-tao_fd_test`, `-tao_test_gradient`)
3. Disable the finite-difference gradient for the solve itself (`-tao_fd_gradient false`) so that TAO uses the hand-coded adjoint gradient during optimization. Without this, TAO would compare the finite-difference gradient against itself.
4. Use unit line search to minimize extra solves (`-tao_ls_type unit`)
5. Print the gradient comparison (`-tao_test_gradient_view`)

After running, the tester parses the relative max-norm from the TAO output. If TAO prints
multiple gradient comparisons (e.g. one per optimization iteration), the tester
checks every one of them and fails if any single comparison exceeds the tolerance.
If `only_first_gradient` is set to `true`, only the first gradient comparison is checked:

- +Max-norm+: `||G - Gfd||/||G||`, the relative max-norm of the difference between the
  hand-coded and finite-difference gradients, normalized by the gradient magnitude.
  Should be near zero for a correct gradient, confirming the adjoint gradient has
  the correct magnitude. The test checks that this value is less than `max_rel_tol`.

## Example test configuration in the MOOSE test suite

In this example, `TaoGradientTester` is used to verify the adjoint gradient for a
point load inversion problem:

!listing modules/optimization/test/tests/executioners/basic_optimize/tests block=debug/gradient_test
         id=testBlock
         caption=Example TaoGradientTester configuration in a tests file.
