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

  [TimeStepper]
    type = SolutionTimeAdaptiveDT
    dt = 0.5
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
