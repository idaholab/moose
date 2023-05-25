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
  num_steps = 10
  # Use the same test case as SolutionTimeAdaptiveDT test, add one more time stepper
  # to test if SolutionTimeAdaptiveDT works correctly with time stepper composition
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
  []
[]

[Postprocessors]
  [dt]
    type = TimestepSize
  []
[]

[Outputs]
  csv = true
  file_base='SolutionTimeAdaptiveDT'
[]
