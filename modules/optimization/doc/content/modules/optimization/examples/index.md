# Example Problems

## Inverse Optimization

The following pages provide examples on how to use the optimization module to solve inverse optimization problems for force and material inversion.  The MOOSE optimization module relies heavily on [MultiApps](MultiApps/index.md) to solve PDE constrained optimization problems.  The main-app in the `Multiapps` system contains the optimization executioner which controls the execution and transfer of data in the sub-apps containing either the "forward" or "adjoint" numerical problem. The "forward" model is the numerical model of the system of interest and is used for computing the objective function.  The "adjoint" model is the adjoint of the forward model and computes the adjoint variable used to compute the derivatives needed for optimization.  Examples on the below pages follow the derivations given on the [Theory page](theory/InvOptTheory.md).

- [examples/forceInv_main.md]
- [examples/materialInv_main.md]
- [examples/constraintOptimization.md]
- [debuggingHelp.md]

In this section, we explore optimization techniques with a particular focus on
two main types: inverse optimization and design optimization. Inverse
optimization primarily involves minimizing the discrepancy between simulated and
experimental data, often parameterizing aspects like force loads or material
properties. This is elaborated upon in this
[section](theory/InvOptTheory.md#sec:adjoint) from the optimization theory page
and in the theory section of the heat conduction MOOSE module. On the other
hand, design optimization involves strategies such as shape and topology
optimization, where the objective is to minimize a general function like strain
energy or maximum stress, which is demonstrated in the [shapeOpt_Annulus.md]


Regardless of which type of inversion problem you are trying to solve, it is recommended that you start with the [Example 1: Point Loads](forceInv_pointLoads.md).  This example contains the best documentation on all of the MOOSE objects needed to solve an optimization and most importantly, this is the simplest optimization problem.  When setting up your own optimization problems, you will likely run into problems and the [debuggingHelp.md] page will help with the Tao executioner options and output that can help locate the problem.

## Topology Optimization

Solid Isotropic Material Penalization (SIMP) examples typically require mechanical or thermal physics to meaningfully leverage
the optimization module's capabilities. Some SIMP examples, including those that combine multiple loads, multiple materials, and
multiple physics are available in the MOOSE combined module.

- [examples/top_opt_main.md]

