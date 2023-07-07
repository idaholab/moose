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
  []
  [check_times]
    type = SimulationTimes
  []
[]

[Problem]
  solve = false
[]

[Executioner]
  type = Transient
  # Test recover
  num_steps = 2

  [TimeStepper]
    type = TimeSequenceFromTimes
    times = input
  []
[]

[Outputs]
  [out]
    type = JSON
    execute_system_information_on = none
  []
[]
