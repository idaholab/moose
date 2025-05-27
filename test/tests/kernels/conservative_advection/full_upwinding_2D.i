# 2D test of advection with full upwinding
# Note there are no overshoots or undershoots
# but there is numerical diffusion.
# The center of the blob advects with the correct velocity

!include no_upwinding_2D.i

[Kernels]
  [udot]
    type := MassLumpedTimeDerivative
  []
  [advection]
    upwinding_type = full
  []
[]
