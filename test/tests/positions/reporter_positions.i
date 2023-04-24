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
    positions = '0 0 1
                 1 0 2'
    outputs = none
  []
  [input_2]
    type = InputPositions
    positions = '0.1 0.1 1
                 1.2 0 2.2'
    outputs = none
  []
  [reporter]
    type = ReporterPositions
    reporters = 'input/positions_1d input_2/positions_1d'
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
