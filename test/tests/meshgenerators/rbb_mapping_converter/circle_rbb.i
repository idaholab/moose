[Mesh]
  [sphere]
    type = SphereMeshGenerator
    radius = 1.0
    nr = 0
    elem_type = QUAD9
  []
  [rbb]
    type = RBBMappingConverter
    input = sphere
  []
[]

[Executioner]
  type = Steady
  [Quadrature]
    # Manual quadrature order, or we default to 0 with no variables and
    # our VolumePostprocessor is stuck with that
    order = FIFTH
  []
[]

[Postprocessors]
  [volume]
    type = VolumePostprocessor
  []
[]

[Problem]
  solve = false
[]

[Outputs]
  [output]
    type = CSV
  []
[]
