[Mesh/gmg]
  type = GeneratedMeshGenerator
  dim = 1
[]

[Postprocessors/value]
  type = ConstantPostprocessor
  value = 2
  execute_on = INITIAL
[]

[Problem]
  solve = False
[]

[Executioner]
  type = Steady
[]

[Outputs]
  csv = true
[]
