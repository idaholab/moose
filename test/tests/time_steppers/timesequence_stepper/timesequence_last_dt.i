[Mesh]
  type = GeneratedMesh
  dim = 1
[]

[Problem]
  solve = false
[]

[Executioner]
  type = Transient
  num_steps = 6
  [TimeStepper]
    type = TimeSequenceStepper
    time_sequence = '0 1 3 4 8'
    use_last_dt_after_last_t = true
  []
[]

[Postprocessors]
  [dt]
    type = TimestepSize
  []
[]

[Outputs]
  csv = true
[]
