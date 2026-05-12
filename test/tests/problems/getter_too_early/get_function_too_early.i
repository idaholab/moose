[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 1
    nx = 1
  []
[]

[Problem]
  type = GetterTooEarlyProblem
  getter = function
  function = my_func
  solve = false
[]

[Functions]
  [my_func]
    type = ParsedFunction
    expression = 'x'
  []
[]

[Executioner]
  type = Transient
  num_steps = 0
[]

[Outputs]
  console = false
[]
