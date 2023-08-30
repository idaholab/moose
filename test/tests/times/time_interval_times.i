[Mesh]
  [cmg]
    type = CartesianMeshGenerator
    dx = 1
    dim = 1
  []
[]

[Times]
  [times]
    type = TimeIntervalTimes
    time_interval = 2.0
    always_include_end_time = true
  []
[]

[Problem]
  solve = false
[]

[Executioner]
  type = Transient
  start_time = 5.0
  end_time = 10.0
  num_steps = 2
[]

[Outputs]
  file_base = 'time_interval_with_end'
  [out]
    type = JSON
    execute_system_information_on = none
  []
[]
