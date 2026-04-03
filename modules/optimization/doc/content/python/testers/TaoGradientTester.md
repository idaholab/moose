# TaoGradientTester

The `TaoGradientTester` compares hand-coded (adjoint) gradients against
finite-difference gradients using TAO's built-in gradient testing functionality.
It automatically injects the necessary PETSc options into the optimization
Executioner (see [executionerBlock]) and parses the output to verify the gradient
is accurate within a specified tolerance.

Only the first computed gradient is checked; all subsequent gradient comparisons
in the TAO output are ignored. This is because the first gradient is evaluated at
the initial parameter values where gradient magnitudes are typically large, making
the relative finite-difference comparison meaningful. Later gradients are evaluated
near convergence where the gradient magnitude is small, causing the relative
finite-difference error to be artificially large.

This tester is useful for verifying that adjoint-based gradient computations are
correct in optimization problems. See the [debugging help](optimization/examples/debuggingHelp.md)
page for more information on debugging optimization gradients.

## Options

Test configuration options are specified in the `tests` file, see [testBlock].

- `max_rel_tol`: Tolerance for the relative max-norm `||G - Gfd||/||G||`, where
  `G` is the hand-coded gradient and `Gfd` is the finite-difference gradient.
  Should be near zero for a correct gradient. Defaults to 1e-5.

- `tao_fd_delta`: Finite-difference step size (`-tao_fd_delta`) used by TAO when
  computing the finite-difference gradient. If not specified, TAO uses its default.
  This parameter is problem dependent and may need to be adjusted for problems with
  large or small parameter values.

The tester automatically sets the following:

- `tao_solver = taobncg` (bounded nonlinear conjugate gradient). This gradient-based
  solver requires only one gradient evaluation per optimization iteration, minimizing
  the number of forward and adjoint solves needed for the test.
- `recover = false` and `restep = false`
- Valgrind is disabled
- Requires `method=opt` (optimized build)

Other test commands and restrictions may be found in the MOOSE TestHarness documentation.

## How It Works

The tester automates the manual gradient testing setup that would otherwise need to be
added directly to the `Executioner` block of an input file, such as:

!listing modules/optimization/test/tests/executioners/basic_optimize/debug_gradient.i block=Executioner
         id=executionerBlock
         caption=Example Executioner block for manual gradient testing with TAO.

Running the input file above produces the following output comparing the finite-difference
gradient `Gfd` to the adjoint gradient `G`:

!listing modules/optimization/test/tests/executioners/basic_optimize/gold/debug_gradient.out start=||Gfd|| end=TAO SOLVER include-end=True

The tester prepends CLI arguments to override the input file's `Executioner` block,
configuring TAO to:

1. Run only a single optimization iteration (`-tao_max_it 1`)
2. Enable finite-difference gradient testing (`-tao_fd_test`, `-tao_test_gradient`)
3. Disable the finite-difference gradient for the solve itself (`-tao_fd_gradient false`),
   so that TAO compares the hand-coded adjoint gradient against the finite-difference
   gradient. Without this, TAO would compare the finite-difference gradient against itself.
4. Use a unit line search to avoid extra solves (`-tao_ls_type unit`)
5. Print the gradient comparison (`-tao_test_gradient_view`)

After running, the tester parses the relative max-norm `||G - Gfd||/||G||` from the
first gradient comparison in the TAO output and checks that it is less than `max_rel_tol`.
A value near zero confirms the adjoint gradient has the correct magnitude. Any subsequent
gradient comparisons in the output are ignored.

## Example test configuration in the MOOSE test suite

!listing modules/optimization/test/tests/executioners/basic_optimize/tests block=debug/gradient_test
         id=testBlock
         caption=Example TaoGradientTester configuration in a tests file.
