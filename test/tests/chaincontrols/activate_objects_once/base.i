[Mesh]
  [gen_mg]
    type = GeneratedMeshGenerator
    dim = 1
    nx = 1
  []
[]

[Functions]
  [test_fn]
    type = ParsedFunction
    expression = 't + 5'
  []
[]

[Problem]
  solve = false
[]

[Executioner]
  type = Transient
  dt = 1
  num_steps = 3
[]

[Outputs]
  csv = true
[]
