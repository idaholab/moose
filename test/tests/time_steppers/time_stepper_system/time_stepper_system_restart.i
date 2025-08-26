[Mesh]
  type = GeneratedMesh
  dim = 1
[]

[Problem]
  solve = false
[]

[Executioner]
  type = Transient
  solve_type = NEWTON
  num_steps = 5

  [TimeSteppers]
    [SolutionTimeAdaptiveDT]
      type = SolutionTimeAdaptiveDTTest
      dt = 0.5
      fake_wall_time_sequence = '100 100 200 200 600 300'
    []

    [LogConstDT]
      type = LogConstantDT
      log_dt = 0.2
      first_dt = 0.1
    []

    [Timesequence]
      type = TimeSequenceStepper
      time_sequence = '0  0.12 0.2 0.5 0.6'
    []
  []
[]

[Postprocessors]
  [dt]
    type = TimestepSize
  []
[]

[Outputs]
  [checkpoint]
    type = Checkpoint
    num_files = 5
  []
  file_base = 'restart_test'
[]
