# FunctionIntegralRayKernel

!syntax description /RayKernels/FunctionIntegralRayKernel

The integration performed for the function $f(r)$ is

!equation
\int_L f(r)~dr, \quad L = \{\vec{r}_1 + t\vec{r}_2 \mid t \in [0, 1]\}\,,

which is achieved by overriding `computeQpIntegral()` as:

!listing modules/ray_tracing/src/raykernels/FunctionIntegralRayKernel.C re=Real\sFunctionIntegralRayKernel::computeQpIntegral.*?^}

The resulting integrated value can be obtained using a [RayIntegralValue.md] postprocessor.

!syntax parameters /RayKernels/FunctionIntegralRayKernel

!syntax inputs /RayKernels/FunctionIntegralRayKernel

!syntax children /RayKernels/FunctionIntegralRayKernel
