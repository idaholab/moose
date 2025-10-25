[Mesh]
  [fmg]
    type = FileMeshGenerator
    file = 'gold/double_hex8_in.e'
  []
  [trans]
    type = BoundaryElementConversionGenerator
    input = fmg
    boundary_names = 'internal'
    conversion_element_layer_number = 2
  []
[]

[Executioner]
  type = Steady
[]

[Postprocessors]
  [v0]
    type = VolumePostprocessor
    block = 0
  []
  [v1]
    type = VolumePostprocessor
    block = 1
  []
  [v2]
    type = VolumePostprocessor
    block = 2
  []
[]

[Problem]
  solve = false
[]

[Outputs]
  csv = true
  execute_on = 'TIMESTEP_END'
[]
