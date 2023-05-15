# Material Inversion Example: Transient solve

## Background

The MOOSE optimization module provides a flexible framework for solving inverse optimization problems in MOOSE.  This page is part of a set of examples for different types of inverse optimization problems.

- [Theory](theory/InvOptTheory.md)
- [Examples overview](optimization/examples/index.md)
- [Example 1: Convective Boundary Conditions](materialInv_ConvectiveBC.md)
- [Example 2: Constant Thermal Conductivity](materialInv_ConstK.md)
- [Example 3: Transient Solve with Automatic Adjoint](material_transient.md)
- [debuggingHelp.md]

# Example: Transient Solve with automatic adjoing problem id=sec:TransientSolveAutomaticAdjoint

In this example, material parameters are optimized within a transient solve setting. This is accomplished
by leveraging inverse optimization theory, see [section](theory/InvOptTheory.md#sec:material_inversion). As
with [material inversion with constant conductivity](materialInv_ConstK.md), this is a nonlinear optimization
problem that requires an appropriate initial guess and parameter bounds to obtain convergence.

## Transient Problem Definition

The problem analyzed here is a two-dimensional diffusion example with square geometry. The right
and top boundaries are subjected to zero Dirichlet boundary conditions. The spatial distribution of
the material parameter $D$ is given in [diff_parameter]. While this material parameter is known in the forward
problem, its distribution to minimize the solution misfit is optimized in the adjoint pass.


!media large_media/optimization/material_transient_diff.png
       style=width:40%;margin:auto;padding-top:2.5%;background-color: white;color: black;
       id=diff_parameter
       caption=Spatial parameterization of the diffusivity parameter for the transient problem.

As this is a transient or non-steady problem, the solution at each time step depends on the solution
at previous steps. The forward problem is solved as usual in MOOSE, using implicit Euler time stepping.
The solution to the diffusion problem is obtained for ten steps with a time step of 0.1 seconds. The
solution to the diffused variable at time $t=0.6$ s, together with the measurement points provided for
material inversion optimization, is shown in [solution].

!media large_media/optimization/material_transient_solution.png
       style=width:40%;margin:auto;padding-top:2.5%;background-color: white;color: black;
       id=solution
       caption=Snaphot of the forward simulation at the end of the sixth time step (0.6 s). The yellow
       marking points to the measurement points provided in the procedure for obtaining
       the optimal diffusivity parameters of the material at the end of the adjoint problem.

The solution to the adjoint problem requires an initial solution vector with estimates of the material
property values. In addition to that, bounds for those parameter need to be provided. In [sensitivity],
we plot the objective function as we vary the initial estimate for the diffusion parameter at the location
(0.75, 0.75). The slope of the objective function with respect to variations in the parameter can have
implication in the overall convergence of the adjoint problem.

!media large_media/optimization/material_transient_of.png
       style=width:40%;margin:auto;padding-top:2.5%;background-color: white;color: black;
       id=sensitivity
       caption=Sensitivity of the objective function when changing the diffusivity parameter at
       the location (0.75, 0.75) around its optimal value '0.05'.


## Problem Input File Setup

This example makes use of the [TransientAndAdjoint](TransientAndAdjoint.md) executioner, which coordinates
the execution of the forward and adjoint problem, freeing the user of the need to set up a multi-app problem
with two child applications corresponding to the forward and adjoint problems (see [Multi-app executioner](materialInv_ConstK.md#sec:MultiAppExecutioner) for an example set up with multi apps).

The entire inverse optimization strategy is governed by the [OptimizationReporter](OptimizationReporter.md) object.
The forward and adjoint problems are set up in an input file that is solved as a `FullSolveMultiApp` type of
`MultiApps` problem. `MultiAppReporterTransfer` objects to move the measurement and simulation data throughout
space and time from and to the forward problem are also set up in the driver input file.

!listing examples/materialTransient/optimize_auto_adjoint.i

One single input file is used to simulation the physics for the optimization problem. The core of the input
file contains the definition of the physics that the user wants to model, as with most of MOOSE's
simulations.
In this case, a diffusion problem with Dirichlet boundary conditions is built. The forward and adjoint problems are run in two separate [nonlinear systems](NonlinearSystem.md), one for the forward problem and another one for the adjoint problem.
These nonlinear systems are inputs to an executioner that drives the simulation of both problems, i.e. the [TransientAndAdjoint](TransientAndAdjoint.md) MOOSE object.

!listing examples/materialTransient/forward_and_adjoint.i block=Problem

!listing examples/materialTransient/forward_and_adjoint.i block=Variables

For material inversion, the derivative of the objective function with respect to the material
parameters needs to be computed. This quantity is problem-dependent. For this diffusion case,
the vector posprocessor `ElementOptimizationDiffusionCoefFunctionInnerProduct` computes the inner
product between the gradient of the adjoint variable and the gradient of the primal variables.

!listing examples/materialTransient/forward_and_adjoint.i block=VectorPostprocessors

