# PetscJacobianTester

The `PetscJacobianTester` tester uses the `-snes_test_jacobian` command line option
in PETSc to evaluate the quality of the Jacobian in the specified tests.

## Options

Test configuration options are specified in the `tests` file.

- `ratio_tol`: Relative tolerance to compare the ration against, defaults to 1e-8

- `difference_tol`: Relative tolerance to compare the difference against, defaults to 1e-8

- `state`: The state for which we want to compare against the
         finite-differenced Jacobian ('user' (default) 'const_positive' or
         'const_negative'.)

- `run_sim`: Whether to actually run the simulation, testing the Jacobian
          at every non-linear iteration of every time step. This is only
          relevant for petsc versions >= 3.9. Defaults to False.

- `turn_off_exodus_output`: Whether to set exodus=false in Outputs. Defaults to True

- `only_final_jacobian`: Check only final Jacobian comparison. Defaults to False


Other test commands & restrictions may be found in the [TestHarness documentation](TestHarness.md).

## Example test configuration in the MOOSE test suite

In this example, four `PetscJacobianTester` and two `Exodiff` tests are used to check the proper
behavior of automatic differentiation in boundary conditions. The Jacobian tests make
sure the contribution to the Jacobian of the boundary conditions is correct.

!listing test/tests/bcs/ad_bcs/tests
