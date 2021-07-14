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
unity as possible.

### Automatic scaling id=auto-scaling

To address a poor condition number, which can often be the result of poor relative scaling between variables, you
can use MOOSE's automatic scaling feature which will bring different physics Jacobians as close to
unity as possible. To turn on this feature, set the `automatic_scaling`
parameter in the `Executioner` block to `true`. Additionally, if you want to update scaling factors
at every time step then set `Executioner/compute_scaling_once=false`. By default this latter
parameter is set to `true` in order to save computational expense.

By default scaling factors are computed solely based on information from
`compute.*Jacobian` methods. However, residual information or a hybrid of
Jacobian and residual information may also be used by setting the `Executioner`
parameter `resid_vs_jac_scaling_param`. The default value is `0`, signaling pure
Jacobian scaling. Setting a value of `1` requests pure residual scaling. Any
value in-between `0` and `1` represents a hybrid of residual and Jacobian.

Another parameter relevant to automatic scaling is the `scaling_group_variables`
parameter. Variables in a group will have a
single common factor applied to them. Multiple groups can be used with
semicolons separating them. An example of this is shown in the `Executioner`
block example below where residual/Jacobian information from all the
displacement (`disp_`) variables would be used to determine a single scaling
factor for themselves, and similarly the residual/Jacobian information from all
the velocity (`vel_`) variables would be used to determine a single scaling
factor for themselves.

```
[Executioner]
:
:
  automatic_scaling = true
  scaling_group_variables = 'disp_x disp_y disp_z; vel_x vel_y vel_z'
:
:
[]
```

The current implementation of automatic scaling in MOOSE is fairly simple and
proceeds according to these steps:

1. Compute the Jacobians and/or residuals for all `Kernels`, `ScalarKernels`,
   `NodalKernels`, and `FVKernels`
    - If `off_diagonals_in_auto_scaling` is set to `true` in the input file,
      then we aggregate all entries in a Jacobian row into a single row value
      taking the absolute value of any value that is submitted to the
      `SparseMatrix::add` method.
    - If `off_diagonals_in_auto_scaling` is not specified or is set to `false`
      in the input file, then only the diagonal entries of the Jacobian matrix
      are considered
    - Regardless of the value of `off_diagonals_in_auto_scaling` the result of
      this operation is a `NumericVector` of data
2. Examine the vector of Jacobian data or residual vector for every row corresponding to a
   given variable (or group of variables; recall the `scaling_group_variables`
   parameter) and then compute a scaling factor such that the maximum absolute
   value of the Jacobian data vector or residual across all of a variable's (or
   group of variables) rows is unity.
3. If a hybrid of residual and Jacobian scaling has been requested, then a
   single scaling factor is computed from the residual scaling factor and
   Jacobian scaling factor. That computation for each variable (group) is done
   with the following code, where `inverse_scaling_factor` actually denotes that
   it is the reciprocal/inverse of the final applied variable scaling factor:

```c++
inverse_scaling_factors[i] =
    std::exp(_resid_vs_jac_scaling_param * std::log(resid_inverse_scaling_factors[i]) +
             (1 - _resid_vs_jac_scaling_param) * std::log(jac_inverse_scaling_factors[i]));
```


We only execute kernel objects because objects like `BoundaryConditions`,
`DGKernels`, `InterfaceKernels`, and `Constraints` may apply penalty factors or
introduce global constraints for things like Dirichlet conditions or hanging
nodes in mesh adaptivity. We want our scaling factors to represent our physics,
not penalties or constraints.
