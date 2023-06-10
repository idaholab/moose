# Material Inversion Example: Transient solve

## Background

The MOOSE optimization module provides a flexible framework for solving inverse optimization problems in MOOSE.  This page is part of a set of examples for different types of inverse optimization problems.

- [Theory](theory/InvOptTheory.md)
- [Examples overview](optimization/examples/index.md)
- [Example 1: Convective Boundary Conditions](materialInv_ConvectiveBC.md)
- [Example 2: Constant Thermal Conductivity](materialInv_ConstK.md)
- [Example 3: Transient Solve with Automatic Adjoint](material_transient.md)
- [debuggingHelp.md]

# Example: Transient Solve with automatic adjoint problem id=sec:TransientSolveAutomaticAdjoint

In this example, the spatially varying thermal conductivity shown in [diff_parameter] is optimized
to fit time dependent temperature measurements at the points shown by the yellow "x"'s in
[solution]. This is accomplished
by leveraging inverse optimization theory, see [section](theory/InvOptTheory.md#sec:material_inversion). As
with [material inversion with constant conductivity](materialInv_ConstK.md), this is a nonlinear optimization
problem that requires an appropriate initial guess and parameter bounds to obtain convergence.

## Transient Problem Definition

The thermal problem analyzed here is a two-dimensional example with square geometry. The right
and top boundaries are subjected to zero temperature Dirichlet boundary conditions. The thermal
conductivity given by the parameter $D$ is being optimized in the four regions shown in
[diff_parameter].  The actual thermal conductivity distribution shown in [diff_parameter] is being
used with the forward problem to produce synthetic measurement points that will be matched.  In the
actual optimization problem, it is this distribution that we are trying to reproduce.

!media large_media/optimization/material_transient_diff.png
       style=width:40%;margin:auto;padding-top:2.5%;background-color: white;color: black;
       id=diff_parameter
       caption=Actual spatially varying thermal conductivity used to produce synthetic temperature
       data. The optimization simulation is attempting to reproduce this thermal conductivity
       distribution.

As this is a transient or non-steady problem, the solution at each time step depends on the solution
at previous steps. The forward problem is solved as usual in MOOSE, using implicit Euler time stepping.
The solution to the thermal problem is obtained for ten steps with a time step of 0.1 seconds. The
solution for the temperature field at time $t=0.6$ s, together with the points where the measured temperatures are taken, is shown in [solution].

!media large_media/optimization/material_transient_solution.png
       style=width:40%;margin:auto;padding-top:2.5%;background-color: white;color: black;
       id=solution
       caption=Snaphot of the forward simulation at the end of the sixth time step (0.6 s). The
       spatially varying thermal conductivity in [diff_parameter] is optimized to minimize the
       difference between the measured and simulated temperature values at the locations shown by
       the yellow x's.

The optimization module will initialize the parameters being optimized to zero or the lower_bound
for the first optimization iteration. For this problem, an initial thermal conductivity
distribution of zero will not be able to find a thermal conductivity distribution that minimizes the
temperature misfit. To help the optimization algorithm out, we provide an initial guess for the
spatially varying thermal conductivity. In [sensitivity],
we plot the objective function as we vary the thermal conductivity at the
location
(0.75, 0.75). The slope of the objective function with respect to variations in the parameter can
have implication in the overall convergence of the optimization problem.

!media large_media/optimization/material_transient_of.png
       style=width:40%;margin:auto;padding-top:2.5%;background-color: white;color: black;
       id=sensitivity
       caption=Sensitivity of the objective function when changing the diffusivity parameter at
       the location (0.75, 0.75) around its optimal value '0.05'.


## Problem Input File Setup

This example makes use of the [TransientAndAdjoint](TransientAndAdjoint.md) executioner, which automatically computes the adjoint by transposing the forward problem's Jacobian.  This frees the user from having to derive and implement an adjoint input file based on the forward problem kernels.  This also solves the forward and adjoint problems in a single sub-application. An alternative method with two child applications corresponding to the forward and adjoint problems is given in [Multi-app executioner](materialInv_ConstK.md#sec:MultiAppExecutioner).

The entire inverse optimization strategy is governed by the [OptimizationReporter](OptimizationReporter.md) object.
The forward and adjoint problems are set up in the main application input file to be solved as a `FullSolveMultiApp` type of
`MultiApps` problem.  Controllable parameter data is transferred to the `to_forward` sub-app using the `MultiAppReporterTransfer` objects.  Objective and gradient information is then passed back to the optimization main application using another `MultiAppReporterTransfer`.

!listing examples/materialTransient/optimize_auto_adjoint.i

A single input file is used to simulate the physics for the optimization problem. The core of the input
file contains the definition of the physics that the user wants to model, as with most of MOOSE's
simulations.
In this case, a heat diffusion problem with Dirichlet boundary conditions is built. The forward and adjoint problems are run in two separate [nonlinear systems](NonlinearSystem.md), one for the forward problem and another one for the adjoint problem.
These nonlinear systems are inputs to an executioner that drives the simulation of both problems, i.e. the [TransientAndAdjoint](TransientAndAdjoint.md) MOOSE object.

!listing examples/materialTransient/forward_and_adjoint.i block=Problem

!listing examples/materialTransient/forward_and_adjoint.i block=Variables

For material inversion, the derivative of the objective function with respect to the material
parameters needs to be computed. This quantity is problem-dependent.

For this thermal problem, the vector posprocessor [ElementOptimizationDiffusionCoefFunctionInnerProduct](ElementOptimizationDiffusionCoefFunctionInnerProduct.md) computes the gradient of the objective with respect
to the controllable parameter by computing the inner
product between the gradient of the adjoint variable and the gradient of the primal variables and the function
computing the derivative of the material property with respect to the controllable parameter.

!listing examples/materialTransient/forward_and_adjoint.i block=VectorPostprocessors

