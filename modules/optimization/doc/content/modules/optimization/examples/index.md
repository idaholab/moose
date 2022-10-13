# Example Problems

The following pages provide examples on how to use the optimization module to solve inverse optimization problems for force and material inversion.  The MOOSE optimization module relies heavily on [MultiApps](MultiApps/index.md) to solve PDE constrained optimization problems.  The main-app in the `Multiapps` system contains the optimization executioner which controls the execution and transfer of data in the sub-apps containing either the "forward" or "adjoint" numerical problem. The "forward" model is the numerical model, for example FEM, of the system of interest and is used for computing the objective function.  The "adjoint" model is the adjoint of the forward model and computes the adjoint variable which is used to compute the derivatives needed for optimization.  The "adjoint" model is closely related to the "forward" model and for some problems the only difference between the two models are the loads being applied and boundary conditions.  Examples on the below pages follow the derivations given on the [Theory page](theory/InvOptTheory.md).

- [examples/forceInv_main.md]
- [examples/materialInv_main.md]

All of the examples in this section are parameterizing force loads or material properties for steady state
heat conduction in a solid, as described in this [section](theory/InvOptTheory.md#sec:adjoint) from the optimization theory page and this [page](/heat_conduction/index.md) from the heat conduction MOOSE module.
