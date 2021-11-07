[Mesh]
  [cmg]
    type = CartesianMeshGenerator
    dim = 2
    dx = '1'
    dy = '0.5 1.2'
    ix = '1'
    iy = '4 6'
  []
[]

[AuxVariables/volume]
  order = CONSTANT
  family = MONOMIAL
[]

[AuxKernels/volume_aux]
  type = VolumeAux
  variable = volume
  boundary = right
[]

[Problem]
  solve = false
[]

[Executioner]
  type = Steady
[]

[Outputs]
  exodus = true
[]
