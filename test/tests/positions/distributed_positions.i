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
    positions = '0 0 0
                 1 0 0
                 0 1 0
                 1 1 0'
    outputs = none
  []
  [input_2]
    type = InputPositions
    positions = '10 0 0
                 0 10 0'
    outputs = none
  []
  [distributed]
    type = DistributedPositions
    positions = 'input_2 input'
  []
[]

[Problem]
  solve = false
[]

[Executioner]
  type = Steady
[]

[Outputs]
  [out]
    type = JSON
    execute_on = FINAL
    execute_system_information_on = none
  []
[]
