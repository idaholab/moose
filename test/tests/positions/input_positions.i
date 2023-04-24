[Mesh]
  [cmg]
    type = CartesianMeshGenerator
    dx = 1
    dim = 1
  []
[]

[Positions]
  [input]
    type = InputPositions
    positions = '0.1 0 0
                 0.2 0 0.1'
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
