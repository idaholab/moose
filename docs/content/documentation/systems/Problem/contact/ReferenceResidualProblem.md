# ReferenceResidualProblem
!syntax description /Problem/ReferenceResidualProblem

## Description
By default, MOOSE checks convergence using relative and absolute criteria. Once
the residual drops below either an absolute tolerance, or the residual divided
by the initial residual for the current time step drops below a relative
tolerance, the solution is considered converged. This works well for many
problems, but in cases where the solution changes minimally between time steps,
the initial residual can be very small, making the relative convergence check
much too stringent.

The `ReferenceResidualProblem` checks for convergence by comparing the residual
to a different reference quantity (instead of the initial residual). The the
user supplies a reference quantity in the form of a set of `AuxVariables` that
contain physically meaningful quantities that can be used in a relative
convergence check. Because the solution variables can have significantly
different scaling, the convergence check performed in `ReferenceResidualProblem`
checks convergence of the solution variables individually. When the $l^2$ norm
of the residual for each solution variable is less than either the relative
tolerance times the $l^2$ norm of the corresponding reference variable or the
absolute tolerance, the solution is considered converged.

Use of this procedure requires that the user provide physically meaningful
reference quantities. The vector of the reaction loads (in the case of
mechanics) or integrated fluxes (in the case of diffusion problems) is
typically suitable for this purpose, as it provides a measure of the loading
applied to the system. To compute these, an AuxVariable must be set up
corresponding to each solution variable, and the save_in option is used in
each Kernel to assemble the residual into that vector. When the solution is
converged, that AuxVariable will contain values that are close to zero
everywhere except for where boundary conditions are applied.

Since relative convergence is computed differently with this approach, the
nonlinear relative tolerance required to achieve the same error is typically
different than with the default approach in MOOSE, and the differences will
vary by the problem. The code user must evaluate the behavior of their model to
ensure that appropriate tolerances are being used.

## Example Input syntax
!listing /modules/combined/test/tests/reference_residual/reference_residual.i block=Problem

where additional AuxVariables for the displacements (e.g. x, y, and z) and temperature must be created, as shown below
!listing /modules/combined/test/tests/reference_residual/reference_residual.i block=AuxVariables

and then called in the respective kernels. For the displacement reference residual `saved` variables, the option `save_in` is set in the Tensor Mechanics Action
!listing /modules/combined/test/tests/reference_residual/reference_residual.i start=Modules/TensorMechanics/Master end=Kernels

and the temperature `saved` variable is applied in the heat conduction kernel
!listing /modules/combined/test/tests/reference_residual/reference_residual.i start=Kernels end=Functions

!syntax parameters /Problem/ReferenceResidualProblem

!syntax inputs /Problem/ReferenceResidualProblem

!syntax children /Problem/ReferenceResidualProblem
