[Mesh/gmg]
  type = GeneratedMeshGenerator
  dim = 1
  nx = 1
[]

[Postprocessors/constant]
  type = ConstantPostprocessor
  value = 5
  execute_on = initial
[]

[Problem]
  solve = false
[]

[Executioner]
  type = Steady
[]

[Outputs]
  csv = true
  checkpoint = true
[]
