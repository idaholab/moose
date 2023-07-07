[Mesh]
  [cmg]
    type = CartesianMeshGenerator
    dx = 1
    dim = 1
  []
[]

[Times]
  [functor]
    type = FunctorTimes
    functor = 'f1'
  []
[]

[Functions]
  [f1]
    type = ParsedFunction
    expression = '1 + x * 2'
  []
[]

[Problem]
  solve = false
[]

[Executioner]
  type = Transient
  # Test recover
  num_steps = 2
[]

[Outputs]
  [out]
    type = JSON
    execute_system_information_on = none
  []
[]
