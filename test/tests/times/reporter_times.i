[Mesh]
  [cmg]
    type = CartesianMeshGenerator
    dx = 1
    dim = 1
  []
[]

[Times]
  [input]
    type = InputTimes
    times = '0.2 0.4 0.9'
    outputs = none
  []
  [input_2]
    type = InputTimes
    times = '0.2 0.5 0.6'
    outputs = none
  []
  [reporter]
    type = ReporterTimes
    reporters = 'input/times input_2/times'
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
