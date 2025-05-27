# ConservativeAdvection with upwinding_type = full
# Apply a velocity = (1, 0, 0) and see a pulse advect to the right
# Note that the pulse diffuses more than with no upwinding,
# but there are no overshoots and undershoots and that the
# center of the pulse at u=0.5 advects with the correct velocity

!include no_upwinding_1D.i

[Kernels]
  [udot]
    type := MassLumpedTimeDerivative
  []
  [advection]
    velocity = '1 0 0'
    upwinding_type = full
  []
[]
