# IntegralRayKernel

## Description

`IntegralRayKernel` is the base class for integrating a quantity $f(r)$ along a [Ray.md] and accumulating the integral into a data member on the [Ray.md]. The integral computed is:

!equation
\int_L f(r)~dr, \quad L = \{\vec{r}_1 + t\vec{r}_2 \mid t \in [0, 1]\}\,,

where $f(r)$ is integrated from $\vec{r}_1$ to $\vec{r}_2$.

!alert note
`IntegralRayKernel` is *not* to be used for contributing to a residual or Jacobian along a [Ray.md]. For this case, you should derive from [RayKernel.md] or [ADRayKernel.md].

For examples, see:

- [VariableIntegralRayKernel.md] for integrating a [Variable](Variables/index.md) or an [AuxVariable](AuxVariables/index.md)
- [MaterialIntegralRayKernel.md] for integrating a [Material](Materials/index.md)
- [FunctionIntegralRayKernel.md] for integrating a [Function](Functions/index.md)

To integrate along a desired quantity, inherit from IntegralRayKernel and override the `computeQpIntegral()` method in which `_qp` is the current quadrature point index. For example:

!listing modules/ray_tracing/src/raykernels/VariableIntegralRayKernel.C re=Real\sVariableIntegralRayKernel::computeQpIntegral.*?^}

Many other useful member variables exist that describe the [Ray.md] segment. For more information, see [Using a RayKernel](syntax/RayKernels/index.md#using-a-raykernel).

The integrated value produced by a class that derives from this should be obtained using the [RayIntegralValue.md] postprocessor. For example:

!listing test/tests/raykernels/variable_integral_ray_kernel/variable_integral_ray_kernel.i start=RayKernels end=Problem
