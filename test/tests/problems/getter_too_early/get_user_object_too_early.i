[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 1
    nx = 1
  []
[]

[Problem]
  type = GetterTooEarlyProblem
  getter = user_object
  user_object = test_uo
  solve = false
[]

[UserObjects]
  [test_uo]
    type = MTUserObject
  []
[]

[Executioner]
  type = Transient
  num_steps = 0
[]

[Outputs]
  console = false
[]
