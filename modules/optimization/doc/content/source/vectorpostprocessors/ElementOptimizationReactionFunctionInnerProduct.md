# ElementOptimizationReactionFunctionInnerProduct

!syntax description /VectorPostprocessors/ElementOptimizationReactionFunctionInnerProduct

## Overview

This vector-postprocessor computes the inner product of two variables with the derivative of an `OptimizationFunction` like [NearestReporterCoordinatesFunction.md]. For steady-state problems, the inner product is defined as:

!equation
V_i = \left(u,-\frac{df}{dp_i}v\right) = \int_{\Omega}u(\vec{r})\cdot-\left.\frac{df(\vec{r},\vec{p})}{dp_i}\right|_{\vec{p}=\vec{p}_0}v(\vec{r})d\vec{r},

which uses a quadrature rule to perform the integration. $u$ and $v$ are the variables specified by [!param](/VectorPostprocessors/ElementOptimizationReactionFunctionInnerProduct/variable) and [!param](/VectorPostprocessors/ElementOptimizationReactionFunctionInnerProduct/forward_variable), respectively. $f(\vec{r}, \vec{p})$ is the `OptimizationFunction` specified by [!param](/VectorPostprocessors/ElementOptimizationReactionFunctionInnerProduct/function), $\vec{p}$ is the vector of parameters that is defined in the function, and $\vec{p}_0$ is the current values of the parameters in the function.

## Example Input File Syntax

This function is primarily used for computing the gradient in an optimization routine where the value of a reaction coefficient is being optimized. See [diffusion_reaction/forward_and_adjoint.i] as an example.

!syntax parameters /VectorPostprocessors/ElementOptimizationReactionFunctionInnerProduct

!syntax inputs /VectorPostprocessors/ElementOptimizationReactionFunctionInnerProduct

!syntax children /VectorPostprocessors/ElementOptimizationReactionFunctionInnerProduct
