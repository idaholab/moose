# ElementOptimizationSourceFunctionInnerProduct

!syntax description /VectorPostprocessors/ElementOptimizationSourceFunctionInnerProduct

## Overview

This vectorpostprocessor computes inner product of a variable with the derivative of an `OptimizationFunction` like [NearestReporterCoordinatesFunction.md]. For steady-state problems, the inner product is defined as:

!equation
V_i = \left(u,\frac{df}{dp_i}\right) = \int_{\Omega}u(\vec{r})\left.\frac{df(\vec{r},\vec{p})}{dp_i}\right|_{\vec{p}=\vec{p}_0}d\vec{r},

which uses a quadrature rule to perform the integration. $u$ is the variable specified by [!param](/VectorPostprocessors/ElementOptimizationSourceFunctionInnerProduct/variable), and $f(\vec{r}, \vec{p})$ is the `OptimizationFunction` specified by [!param](/VectorPostprocessors/ElementOptimizationSourceFunctionInnerProduct/function), $\vec{p}$ is the vector of parameters that is defined in the function, and $\vec{p}_0$ is the current values of the parameters in the function. For transient problems:

!equation
V_i = \left(u,\frac{df}{dp_i}\right) = \int_{t_0}^{t_N}\int_{\Omega}u(\vec{r},t)\left.\frac{df(\vec{r},t,\vec{p})}{dp_i}\right|_{\vec{p}=\vec{p}_0}d\vec{r}dt,

where $t_0$ and $t_N$ are the initial and final time points, respectively. The time integration is done using a trapezoid rule:

!equation
V_i \approx \sum_{n=0}^{N-1}\frac{V^{r}_i(t_{n+1}) + V^{r}_i(t_{n})}{2}\left(t_{n+1} - t_{n}\right),

where,

!equation
V^r_i(t) = \int_{\Omega}u(\vec{r},t)\left.\frac{df(\vec{r},t,\vec{p})}{dp_i}\right|_{\vec{p}=\vec{p}_0}d\vec{r}

!syntax parameters /VectorPostprocessors/ElementOptimizationSourceFunctionInnerProduct

!syntax inputs /VectorPostprocessors/ElementOptimizationSourceFunctionInnerProduct

!syntax children /VectorPostprocessors/ElementOptimizationSourceFunctionInnerProduct
