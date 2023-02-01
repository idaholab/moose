# Material Inversion Example: Convective Boundary Condition

## Background

The MOOSE optimization module provides a flexible framework for solving inverse optimization problems in MOOSE.  This page is part of a set of examples for different types of inverse optimization problems.

- [Theory](theory/InvOptTheory.md)
- [Examples overview](optimization/examples/index.md)
- [Example 1: Convective Boundary Conditions](materialInv_ConvectiveBC.md)
- [Example 2: Constant Thermal Conductivity](materialInv_ConstK.md)
- [debuggingHelp.md]

# Example: Convective Boundary Condition id=sec:ConvectiveBCMaterialInversion

In this example, the convective coefficient from this equation [!eqref](theory/InvOptTheory.md#eq:robin_bc_types)
is fit to experimental data.  This is a nonlinear optimization problem but is limited to a boundary condition and will be easier to solve than material property inversion presented in [Example 2: Constant Thermal Conductivity](materialInv_ConstK.md).  This problem consists of a rectangular domain where the convective BC is applied to on the left.  The top and bottom BCs are Dirichlet and the right BC is Neumann.  This is a nonlinear optimization problem where the design parameters show up in the derivative of the PDE, see this [section](theory/InvOptTheory.md#sec:robinInv) from the theory page.  Getting a nonlinear optimization problem to converge is dependent on the initial guess and bounds for the design parameters.  Convergence is limited to local minima and doesn't guarantee a global minimum.  All of this makes material inversion problems more difficult to solve than the linear optimization problems for [force inversion](theory/InvOptTheory.md#sec:forceInv).

## Main-App Optimization Executioner

The main input file containing the optimization reporter, executioner and transfers is shown in [main].  The adjoint problem will need the simulation temperature from the forward problem to evaluate [!eqref](theory/InvOptTheory.md#eq:convectiveBC) for the convective BC.  This requires us to transfer the forward simulation temperature field to the adjoint-app.

!listing modules/combined/test/tests/invOpt_bc_convective/main.i id=main

## Forward Problem Sub-App

The forward input file containing the solution to the PDE of interest is shown in [forward].  The optimization executioner is controlling the `coefficient` in `[ConvectiveFluxFunction]` boundary condition.  The coefficient is controlled by transferring data from the main-app into a chain of objects on the forward-app.  The main-app first transfers the convection coefficient into a `[ConstantValuePostprocessor]` on the forward-app, which is then transferred into a function that can finally be transferred into the `ConvectiveFluxFunction` BC.

!listing modules/combined/test/tests/invOpt_bc_convective/forward.i id=forward

## Adjoint Problem Sub-App

The adjoint input file shown in [adjoint] computes the adjoint of the forward PDE.  The adjoint problem uses a `Controls` block to allow the main app to transfer the material property used in the forward problem to the adjoint problem.  The temperature field computed in the forward problem was transferred back to the main app and is finally transferred from the main app into the adjoint problem using a `MultiAppCopyTransfer`.  The gradient given by [!eqref](theory/InvOptTheory.md#eq:convectiveBC) is computed on the left boundary using the  [SideIntegralVariablePostprocessor.md] postprocessor.

!listing modules/combined/test/tests/invOpt_bc_convective/adjoint.i id=adjoint
