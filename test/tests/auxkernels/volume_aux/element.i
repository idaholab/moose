[Mesh]
  [cmg]
    type = CartesianMeshGenerator
    dim = 2
    dx = '1.5 2.4 0.1'
    dy = '1.3 0.9'
    ix = '2 1 1'
    iy = '2 3'
  []
[]

[AuxVariables/volume]
  order = CONSTANT
  family = MONOMIAL
[]

[AuxKernels/volume_aux]
  type = VolumeAux
  variable = volume
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
