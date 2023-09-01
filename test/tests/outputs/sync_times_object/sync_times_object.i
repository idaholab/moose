[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 1
[]

[Postprocessors]
  [current_time]
    type = TimePostprocessor
    execute_on = 'INITIAL TIMESTEP_END'
  []
[]

[Problem]
  solve = false
[]

[Executioner]
  type = Transient
  dt = 1
  end_time = 5
[]

[Times]
  [input_times]
    type = InputTimes
    times = '1.1 1.5 2.3'
  []
  # For the error-check test
  [simulation_times]
    type = SimulationTimes
  []
[]

[Outputs]
  [out]
    type = CSV
    sync_only = true
    sync_times_object = input_times
    execute_reporters_on = 'NONE'
  []
[]
