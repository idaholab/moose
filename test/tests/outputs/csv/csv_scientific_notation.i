[Mesh]
  type = GeneratedMesh
  dim = 1
[]

[Postprocessors]
  [small]
    type = ConstantPostprocessor
    value = 1.234e-20
  []
[]

[Problem]
  solve = false
[]

[Executioner]
  type = Steady
[]

[Outputs]
  file_base = 'csv_scientific_notation_out'
  [csv]
    type = CSV
    execute_on = 'TIMESTEP_END'
    precision = 6
    scientific_notation = true
  []
[]
