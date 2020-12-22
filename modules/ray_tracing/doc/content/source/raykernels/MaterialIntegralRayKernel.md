# MaterialIntegralRayKernel

## Description

!syntax description /RayKernels/MaterialIntegralRayKernel

The integral performed for the material $u$ is

!equation
\int_L u~dr, \quad L = \{\vec{r}_1 + t\vec{r}_2 \mid t \in [0, 1]\}\,,

where $u$ is the material property, which is achieved by overriding `computeQpIntegral` as:

!listing modules/ray_tracing/src/raykernels/MaterialIntegralRayKernel.C re=Real\sMaterialIntegralRayKernel::computeQpIntegral.*?^}

The resulting integrated value can be obtained using a [RayIntegralValue.md] postprocessor.

!syntax parameters /RayKernels/MaterialIntegralRayKernel

!syntax inputs /RayKernels/MaterialIntegralRayKernel

!syntax children /RayKernels/MaterialIntegralRayKernel
