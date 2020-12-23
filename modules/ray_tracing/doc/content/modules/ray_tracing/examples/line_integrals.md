# Computing Line Integrals

The [ray_tracing/index.md] can be utilized to integrate fields along a line throughout the domain. Example fields include [Variables](Variables/index.md), [AuxVariables](AuxVariables/index.md),
[Material properties](Materials/index.md), and [Functions](Functions/index.md).

The discussion that follows will describe how to integrate a variable across a line in a simple diffusion problem. To integrate other fields, the process remains the same except for the definition of the RayKernel.

## Problem Definition

We begin with the standard "simple diffusion" problem:

!listing modules/ray_tracing/test/tests/raykernels/variable_integral_ray_kernel/simple_diffusion_line_integral.i start=Mesh end=Outputs

For this problem, we seek the value of the integrals (where $u_h$ is the finite-element solution)

!equation
\int_{L_1} u_h(\vec{r})~dr, \quad L_1 = \{(0, 0) + t(5, 5) \mid t \in [0, 1]\}\,,

and

!equation
\int_{L_2} u_h(\vec{r})~dr, \quad L_2 = \{(5, 0) + t(5, 5) \mid t \in [0, 1]\}\,,

in which we will denote the first line, $L_1$, as `diag` and the second, $L_2$, as `right_up` for simplicity.

Note that the integral along the second line, $L_2$, is trivial due to the Dirichlet boundary condition,

!equation
u_h(5, y) = 1\,, \quad y \in [0, 5]\,,

which implies

!equation
\int_{L_2} u_h(\vec{r})~dr = \int_0^5 u_h(5, y)\,dy = \int_0^5 dy = 5\,, \quad L_2 = \{(5, 0) + t(5, 5) \mid t \in [0, 1]\}\,.

## Defining the Study

A [RepeatableRayStudy.md] is defined that generates and executes the rays that compute the variable line integral:

!listing modules/ray_tracing/test/tests/raykernels/variable_integral_ray_kernel/simple_diffusion_line_integral.i start=UserObjects end=RayKernels

The `study` object defines two rays to be exectued on `TIMESTEP_END`:

- `diag` from $(0, 0)$ to $(5, 5)$
- `right_up` from $(5, 0)$ to $(5, 5)$

## Defining the RayKernel

[RayKernels/index.md] are objects that are executed on the segments of the rays. In this case, we wish to compute the integral of a variable so we will define a [VariableIntegralRayKernel.md]:

!listing modules/ray_tracing/test/tests/raykernels/variable_integral_ray_kernel/simple_diffusion_line_integral.i start=RayKernels end=Postprocessors

The `u_integral` [VariableIntegralRayKernel.md] will accumulate the variable line integral of the `u` Variable for our defined rays, `diag` and `right_up`.

!alert note
Other commonly used [IntegralRayKernels](IntegralRayKernel.md) are the [FunctionIntegralRayKernel.md] and the [MaterialIntegralRayKernel.md].

## Integral Output

Lastly, we need to obtain the accumulated integrals from the `study`. We will utilize a [RayIntegralValue.md] [Postprocessor](Postprocessors/index.md) to achieve this:

!listing modules/ray_tracing/test/tests/raykernels/variable_integral_ray_kernel/simple_diffusion_line_integral.i start=Postprocessors

The accumulated integrals are seen in output:

```
Postprocessor Values:
+----------------+--------------------+------------------------+
| time           | diag_line_integral | right_up_line_integral |
+----------------+--------------------+------------------------+
|   0.000000e+00 |       0.000000e+00 |           0.000000e+00 |
|   1.000000e+00 |       3.535534e+00 |           5.000000e+00 |
+----------------+--------------------+------------------------+
```

## Segment-wise Integral Output

The segment-wise accumulated integral can also be outputted in a mesh format using the [RayTracingMeshOutput.md]. For more information, see [RayTracingMeshOutput.md#example].
