# Example Problems

The following pages provide examples on how to use the optimization module to solve inverse optimization problems for force and material inversion.  The MOOSE optimization module relies heavily on [MultiApps](MultiApps/index.md) to solve PDE constrained optimization problems.  The main-app in the `Multiapps` system contains the optimization executioner which controls the execution and transfer of data in the sub-apps containing either the "forward" or "adjoint" numerical problem. The "forward" model is the numerical model of the system of interest and is used for computing the objective function.  The "adjoint" model is the adjoint of the forward model and computes the adjoint variable used to compute the derivatives needed for optimization.  Examples on the below pages follow the derivations given on the [Theory page](theory/InvOptTheory.md).

- [examples/forceInv_main.md]
- [examples/materialInv_main.md]
- [debuggingHelp.md]

All of the examples in this section are parameterizing force loads or material properties for steady state
heat conduction in a solid, as described in this [section](theory/InvOptTheory.md#sec:adjoint) from the optimization theory page and in the theory section of the heat conduction MOOSE module.

Regardless of which type of inversion problem you are trying to solve, it is recommended that you start with the [Example 1: Point Loads](forceInv_pointLoads.md).  This example contains the best documentation on all of the MOOSE objects needed to solve an optimization and most importantly, this is the simplest optimization problem.  When setting up your own optimization problems, you will likely run into problems and the [debuggingHelp.md] page will help with the Tao executioner options and output that can help locate the problem.
