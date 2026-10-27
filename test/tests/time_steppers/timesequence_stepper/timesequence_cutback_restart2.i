[Mesh]
  file = timesequence_cutback_restart1_out_cp/LATEST
[]

[Problem]
  solve = false
  restart_file_base = timesequence_cutback_restart1_out_cp/LATEST
[]

[Executioner]
  type = Transient
  end_time = 2
  num_steps = 4
  dtmin = 0.25

  [TimeStepper]
    type = TimeSequenceStepper
    time_sequence = '0 0.75 1.5 2'
  []
[]

[Postprocessors]
  [timestep]
    type = TimePostprocessor
    execute_on = timestep_end
  []
[]

[Outputs]
  csv = true
[]
