[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 2
  ny = 2
  xmax = 1
  ymax = 1
[]

[Problem]
  solve = false
[]

[Variables]
  [u]
  []
[]

[Materials]
  [filter]
    type = PackedColumn
    diameter = 2
    viscosity = 1e-03
    output_properties = 'permeability viscosity'
    outputs = exodus
  []
[]

[Executioner]
  type = Steady
[]

[Outputs]
  exodus = true
[]
