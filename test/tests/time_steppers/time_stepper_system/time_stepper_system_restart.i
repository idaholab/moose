[Mesh]
  type = GeneratedMesh
  dim = 1
[]

[Variables]
  [u]
    order = CONSTANT
    family = MONOMIAL
  []
[]

[Problem]
   type = SlowProblem
   seconds_to_sleep = '0.0 0.0 0.1 0.1 0.5 0.2 0.2 0.1 0.1 0.1'
   kernel_coverage_check = false
[]

[Executioner]
  type = Transient
  solve_type = NEWTON
  num_steps = 5

  [TimeSteppers]
    [SolutionTimeAdaptiveDT]
      type = SolutionTimeAdaptiveDT
      dt = 0.5
    []

    [LogConstDT]
      type = LogConstantDT
      log_dt = 0.2
      first_dt = 0.1
    []

    [Timesequence]
      type = TimeSequenceStepper
      time_sequence  = '0  0.12 0.2 0.5 0.6'
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
  file_base='restart_test'
[]
