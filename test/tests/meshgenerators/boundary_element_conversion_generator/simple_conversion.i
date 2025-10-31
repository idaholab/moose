[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 3
    elem_type = HEX8
    nx = 2
    ny = 2
    nz = 2
  []
  [trans]
    type = BoundaryElementConversionGenerator
    input = gmg
    boundary_names = 'left'
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
