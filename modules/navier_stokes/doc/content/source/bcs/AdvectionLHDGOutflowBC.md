# AdvectionLHDGOutflowBC

`AdvectionLHDGOutflowBC` supplies the exterior advective flux for an L-HDG
scalar discretization. The flux uses the hybrid
[!param](/BCs/AdvectionLHDGOutflowBC/face_velocity), consistent with the
interior numerical face flux; the [!param](/BCs/AdvectionLHDGOutflowBC/velocity)
parameter identifies the cell velocity material used by the paired volume
kernel.

Set [!param](/BCs/AdvectionLHDGOutflowBC/constrain_lm) to `true` when advection
must supply the facet-scalar equation. Set it to `false` when an L-HDG diffusion
boundary condition already supplies that trace equation.

!syntax parameters /BCs/AdvectionLHDGOutflowBC

!syntax inputs /BCs/AdvectionLHDGOutflowBC

!syntax children /BCs/AdvectionLHDGOutflowBC
