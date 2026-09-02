[Mesh]
  type = GeneratedMesh
  dim = 1
[]

[Problem]
  solve = false
[]

[Executioner]
  type = Transient
  end_time = 2

  [TimeStepper]
    type = TimeSequenceStepper
    time_sequence = '0 1 2'
  []
[]

[Postprocessors]
  [timestep]
    type = TimePostprocessor
    execute_on = timestep_end
  []
[]

[Outputs]
  [out]
    type = CSV
    sync_times = '0.5'
  []
[]
