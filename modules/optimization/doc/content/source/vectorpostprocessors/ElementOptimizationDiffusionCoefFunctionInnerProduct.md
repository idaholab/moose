# ElementOptimizationDiffusionCoefFunctionInnerProduct

!syntax description /VectorPostprocessors/ElementOptimizationDiffusionCoefFunctionInnerProduct

## Overview

This vector-postprocessor computes the inner product of the gradient of two variables with the derivative of an `OptimizationFunction` like [NearestReporterCoordinatesFunction.md]. For steady-state problems, the inner product is defined as:

!equation
V_i = \left(\vec{\nabla}u,\frac{df}{dp_i}\vec{\nabla}v\right) = \int_{\Omega}\vec{\nabla}u(\vec{r})\cdot\left.\frac{df(\vec{r},\vec{p})}{dp_i}\right|_{\vec{p}=\vec{p}_0}\vec{\nabla}v(\vec{r})d\vec{r},

which uses a quadrature rule to perform the integration. $u$ and $v$ are the variables specified by [!param](/VectorPostprocessors/ElementOptimizationDiffusionCoefFunctionInnerProduct/variable) and [!param](/VectorPostprocessors/ElementOptimizationDiffusionCoefFunctionInnerProduct/forward_variable), respectively. $f(\vec{r}, \vec{p})$ is the `OptimizationFunction` specified by [!param](/VectorPostprocessors/ElementOptimizationSourceFunctionInnerProduct/function), $\vec{p}$ is the vector of parameters that is defined in the function, and $\vec{p}_0$ is the current values of the parameters in the function. 

## Example Input File Syntax

This function is primarily used for computing the gradient in an optimization routine where the value of a diffusion coefficient is being optimized. See [bimaterial/grad.i] as an example.

!syntax parameters /VectorPostprocessors/ElementOptimizationDiffusionCoefFunctionInnerProduct

!syntax inputs /VectorPostprocessors/ElementOptimizationDiffusionCoefFunctionInnerProduct

!syntax children /VectorPostprocessors/ElementOptimizationDiffusionCoefFunctionInnerProduct
