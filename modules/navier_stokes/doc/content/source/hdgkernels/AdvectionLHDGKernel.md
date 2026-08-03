# AdvectionLHDGKernel

`AdvectionLHDGKernel` implements the element-volume and interior-face terms for
an L-HDG scalar advection discretization. The [!param](/HDGKernels/AdvectionLHDGKernel/velocity)
material property is the cell velocity used in the volume integral. The required
[!param](/HDGKernels/AdvectionLHDGKernel/face_velocity) functor is the hybrid
velocity used to choose the upwind scalar and compute the numerical flux on
faces. These velocities should therefore use different finite element spaces with
the latter being appropriate for facet integrals, e.g. something like `SIDE_HIERARCHIC`.

!syntax parameters /HDGKernels/AdvectionLHDGKernel

!syntax inputs /HDGKernels/AdvectionLHDGKernel

!syntax children /HDGKernels/AdvectionLHDGKernel
