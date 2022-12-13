# SideOptimizationNeumannFunctionInnerProduct

!syntax description /VectorPostprocessors/SideOptimizationNeumannFunctionInnerProduct

## Overview

This vector-postprocessor computes the side inner product of a variable with the derivative of an `OptimizationFunction` like [NearestReporterCoordinatesFunction.md]. For steady-state problems, the inner product is defined as:

!equation
V_i = \left(u,\frac{df}{dp_i}\right)_{\partial\Omega} = \oint_{\partial\Omega}u(\vec{r})\left.\frac{df(\vec{r},\vec{p})}{dp_i}\right|_{\vec{p}=\vec{p}_0}d\vec{r},

which uses a quadrature rule to perform the integration. $u$ is the variable specified by [!param](/VectorPostprocessors/SideOptimizationNeumannFunctionInnerProduct/variable), $\partial\Omega$ is the side-set of the domain the integration is taking place specified by [!param](/VectorPostprocessors/SideOptimizationNeumannFunctionInnerProduct/boundary), $f(\vec{r}, \vec{p})$ is the `OptimizationFunction` specified by [!param](/VectorPostprocessors/SideOptimizationNeumannFunctionInnerProduct/function), $\vec{p}$ is the vector of parameters that is defined in the function, and $\vec{p}_0$ is the current values of the parameters in the function.

## Example Input File Syntax

This function is primarily used for computing the gradient in an optimization routine where the value of a [Neumman boundary condtion](FunctionNeumannBC.md) is being optimized. See [bc_load_linearFunction/adjoint.i] as an example.

!syntax parameters /VectorPostprocessors/SideOptimizationNeumannFunctionInnerProduct

!syntax inputs /VectorPostprocessors/SideOptimizationNeumannFunctionInnerProduct

!syntax children /VectorPostprocessors/SideOptimizationNeumannFunctionInnerProduct
