# Material Inversion Example: Constant Thermal Conductivity

## Background

The MOOSE optimization module provides a flexible framework for solving inverse optimization problems in MOOSE.  This page is part of a set of examples for different types of inverse optimization problems.

- [Theory](theory/InvOptTheory.md)
- [Examples overview](optimization/examples/index.md)
- [Example 1: Convective Boundary Conditions](materialInv_ConvectiveBC.md)
- [Example 2: Constant Thermal Conductivity](materialInv_ConstK.md)
- [debuggingHelp.md]

# Example: Constant Thermal Conductivity id=sec:ConstMaterialInversion

In this example, material properties are the design parameters, described in this [section](theory/InvOptTheory.md#sec:material_inversion) on the theory page.  This is a nonlinear optimization problem where the design parameters show up in the derivative of the PDE, see [!eqref](theory/InvOptTheory.md#eq:kappaLambda).  Getting a nonlinear optimization problem to converge is dependent on the initial guess and bounds for the design parameters.  Even convergence doesn't guarantee a correct solution.  All of this makes material inversion problems more difficult to solve than the linear optimization problems for [force inversion](theory/InvOptTheory.md#sec:forceInv).

## Main-App Optimization Executioner

The main input file containing the optimization reporter, executioner and transfers is shown in [main].  The gradient of the PDE given by [!eqref](theory/InvOptTheory.md#eq:kappaLambda) requires the temperature field from the forward problem to be available in the adjoint problem.  This requires the forward problem temperature field be transferred to the adjoint problem.  The adjoint problem also needs to be executed with the same material properties used in the forward problem, see the `toAdjointParameter` transfer in [main].

!listing test/tests/optimizationreporter/material/main.i id=main

## Forward Problem Sub-App

The forward input file containing the solution to the PDE of interest is shown in [forward].  The optimization executioner is controlling the thermal_conductivity `prop_name` in the `thermal` Material block.

!listing test/tests/optimizationreporter/material/forward.i id=forward

## Adjoint Problem Sub-App

The adjoint input file shown in [adjoint] computes the adjoint of the forward PDE.  The adjoint problem uses [ConstantReporter.md] and [ParsedOptimizationFunction.md] objects to allow the main app to transfer the material property used in the forward problem to the adjoint problem.  The temperature field computed in the forward problem was transferred back to the main app and is finally transferred from the main app into the adjoint problem using a [MultiAppCopyTransfer.md].  The gradient given by [!eqref](theory/InvOptTheory.md#eq:kappaLambda) is computed by the [ElementOptimizationDiffusionCoefFunctionInnerProduct](ElementOptimizationDiffusionCoefFunctionInnerProduct.md) vector-postprocessor.

!listing test/tests/optimizationreporter/material/adjoint.i id=adjoint
