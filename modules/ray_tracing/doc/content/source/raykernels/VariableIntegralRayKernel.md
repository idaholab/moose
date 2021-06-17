# VariableIntegralRayKernel

!syntax parameters /RayKernels/VariableIntegralRayKernel

The integral performed along the value $u$ is

!equation
\int_L u~dr, \quad L = \{\vec{r}_1 + t\vec{r}_2 \mid t \in [0, 1]\}\,,

which is achieved by overriding `computeQpIntegral`, as:

!listing modules/ray_tracing/src/raykernels/VariableIntegralRayKernel.C re=Real\sVariableIntegralRayKernel::computeQpIntegral.*?^}

The resulting integrated value can be obtained using a [RayIntegralValue.md] postprocessor.

!syntax parameters /RayKernels/VariableIntegralRayKernel

!syntax inputs /RayKernels/VariableIntegralRayKernel

!syntax children /RayKernels/VariableIntegralRayKernel
