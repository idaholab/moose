# ADConservativeAdvection with upwinding_type = None
# Apply a velocity = (1, 0, 0) and see a pulse advect to the right
# Note there are overshoots and undershoots

!include no_upwinding_1D.i

[Materials]
  [v]
    type := ADGenericConstantVectorMaterial
  []
[]

[Kernels]
  [udot]
    type := ADTimeDerivative
  []
  [advection]
    type := ADConservativeAdvection
  []
[]
