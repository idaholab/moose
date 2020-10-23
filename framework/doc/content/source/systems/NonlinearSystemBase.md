# NonlinearSystemBase

This class derives from [SystemBase.md] and is the immediate base class for
[NonlinearSystem.md] and NonlinearEigenSystem. In general a simulation will
always have exactly one `NonlinearSystemBase` object that holds and operates on
the simulation's nonlinear variables.

## Scaling id=scaling

A critical part to an efficient nonlinear solve is an efficient and accurate
linear solve. If the linear system has a poor condition number, the linear solve
may be inefficient or in the case of PJFNK, may even be
inaccurate. Preconditioning is meant to combat poor linear system conditioning,
but it may not be helpful if the preconditioning matrix, on which the
preconditioning process relies, is itself poorly conditioned. You can inspect
the condition number of your preconditioning matrix for small problems (should be less
than 1000 degrees of freedom) by running with the PETSc options `-pc_type svd
-pc_svd_monitor`. These options will tell you your condition number as well as how many singular
values you have. If you have any singular values, then you may have omitted a boundary condition,
you may have a null space (all Neumann boundary conditions for example), or you may have very poor
scaling between variables in a multi-physics simulation. You may even have run into issues if you
have nodal boundary conditions (which introduce values of unity on the diagonals) and the Jacobian
entries from your physics (kernels) are very large. You want your condition number to be as close to
unity as possible. To address the latter problem or poor relative scaling between variables, you
can use MOOSE's automatic scaling feature which will bring different physics Jacobians as close to
unity as possible. To turn on this feature, set the `automatic_scaling`
parameter in the `Executioner` block to `true`. Additionally, if you want to update scaling factors
at every time step then set `Executioner/compute_scaling_once=false`. By default this latter
parameter is set to `true` in order to save computational expense.
