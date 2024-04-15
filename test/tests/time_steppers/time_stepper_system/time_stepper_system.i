[Mesh]
  file = restart_test_cp/0005-mesh.cpa.gz
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
  restart_file_base = restart_test_cp/0005
[]

[Executioner]
  type = Transient
  solve_type = NEWTON
  num_steps = 10

  [TimeSteppers]
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
  csv = true
  file_base='time_stepper_restart'
[]

