# FEProblemSolve

The `FEProblemSolve` class has two main roles:

- handle a variety of parameters for the [linear and nonlinear solves](systems/NonlinearSystem.md)
- encapsulate the solve function call with a [geometric grid sequencing calls](syntax/Executioner/index.md#Grid Sequencing) for nonlinear problems
- encapsulate each nonlinear system within a multi-system fixed point loop. This loop is nested within the geometric grid sequencing.

The `FEProblemSolve` is a solve executioner nested inside most executioners,
such as [Steady](executioners/Steady.md) and [Transient](executioners/Transient.md) but notably *not* in the [Eigenvalue](executioners/Eigenvalue.md) executioner.

## Multi-system solve capabilities

If using multiple nonlinear systems, the default behavior of the `FEProblemSolve` will be to solve them one by one,
in the order that they were specified, without iterating between systems.

If the [!param](/Executioner/Steady/multi_system_fixed_point) parameter is set to true, this solve will be iterated.
The user must pass a convergence object to the [!param](/Executioner/Steady/multi_system_fixed_point_convergence)
to let the `FEProblemSolve` know when to terminate the fixed point loop.

!alert note
Options are currently limited for setting a multi-system fixed point convergence. We do not recommend using the
nonlinear residual with a [VariableResidual.md] postprocessor or a [DefaultNonlinearConvergence.md] as these
are not re-computed the end of a multi-system fixed point iteration.
