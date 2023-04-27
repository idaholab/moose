# SteadyAndAdjoint

!syntax description /Executioner/SteadyAndAdjoint

## Overview

This executioner can be used to solve a steady-state forward problem with its adjoint. The forward solve is performed the same way as in [Steady.md], but with the [!param](/Executioner/Steady/solve_type) set to `NEWTON`. This performs the nonlinear iteration in the form:

!equation
\mathbf{J}(\mathbf{u}^{i-1}) \delta \mathbf{u}^{i} = \mathbf{R}(\mathbf{u}^{i-1}), i=1,...,I,

where $I$ is the number of iterations it took to converge the problem. The adjoint problem is then defined as:

!equation
\mathbf{J}^{\top}(\mathbf{u}^{I})\lambda = -\mathbf{R}_{\lambda},

where $\lambda$ is the adjoint solution, $\mathbf{R}_{\lambda}$ is the residual of the adjoint system with $\lambda\equiv 0$, and $\mathbf{J}^{\top}(\mathbf{u}^{I})$ is the transpose of the forward system's Jacobian evaluated with the converged forward solution. The adjoint system is basically a linearized and homogenized version of the forward problem with it's own definition of sources.

In order to accurately define the adjoint system, the fully consistent Jacobian must be evaluated. As such, the `computeQpJacobian` routines in the forward problem kernels must be accurately defined or [automatic_differentiation/index.md] must be used. Consider using the [Jacobian debugger](analyze_jacobian.md) to ensure the Jacobian is computed accurately.

## Example Input File Syntax

### Solving Adjoint Problems

The first step is to add an adjoint nonlinear system using the [!param](/Problem/FEProblem/nl_sys_names) parameter in the [Problem](Problem/index.md) input block. It is convenient to define the forward system as `nl0`.

!listing steady_and_adjoint/self_adjoint.i block=Problem

Next we need to add an adjoint variable for each forward variable, which is associated with the `adjoint` system:

!listing steady_and_adjoint/self_adjoint.i block=Variables caption=Single adjoint variable

!listing steady_and_adjoint/multi_variable.i block=Variables caption=Multiple adjoint variables

!listing steady_and_adjoint/array_variable.i block=Variables caption=Array adjoint variables

Next we add kernels and BCs associated with the forward and adjoint variables. Only source-like kernels should be added to the adjoint variables like [BodyForce.md], [ConstantPointSource.md], or [NeumannBC.md].

!listing steady_and_adjoint/nonhomogeneous_bc.i block=Kernels BCs

For nonlinear, problems one should use `AD` Kernels, BCs, and Materials.

!listing steady_and_adjoint/nonlinear_diffusion.i block=Kernels BCs Materials

Finally, we will add this executioner and set the forward/adjoint system tags. Note that the tolerance for the adjoint system solve is set solely by linear solver parameters like [!param](/Executioner/SteadyAndAdjoint/l_tol), [!param](/Executioner/SteadyAndAdjoint/l_abs_tol), and [!param](/Executioner/SteadyAndAdjoint/l_max_its).

!listing steady_and_adjoint/nonhomogeneous_bc.i block=Executioner

### Utilization in Gradient-Based Optimization

Utilizing this executioner for gradient-based optimization is quite powerful since the adjoint used to compute the gradient is automatically assembled with this executioner, given that the forward problem Jacobian can be fully constructed.

To include the source for the adjoint problem, the [ReporterPointSource.md] can be used to add the simulation misfit from the forward solve, which is calculated in [OptimizationData.md].

!listing nonlinear_material/forward_and_adjoint.i block=Reporters DiracKernels

The gradient can then be computed using an inner-product vector-postprocessor like [ElementOptimizationSourceFunctionInnerProduct.md]. Note that these vector-postprocessors must be executed on `ADJOINT_TIMESTEP_END` which occurs after the adjoint system is solved.

!listing nonlinear_material/forward_and_adjoint.i block=VectorPostprocessors

The driving optimize app will thus have only have a single [FullSolveMultiApp.md] which then transfers both the simulation data (for the objective evaluation) and the inner products (for the gradient evaluation).

!listing nonlinear_material/main.i block=MultiApps Transfers

!syntax parameters /Executioner/SteadyAndAdjoint

!syntax inputs /Executioner/SteadyAndAdjoint

!syntax children /Executioner/SteadyAndAdjoint
