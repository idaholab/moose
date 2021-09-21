[Mesh]
  type = GeneratedMesh
  dim = 2
[]

[Variables]
  [u]
  []
[]

[Problem]
  type = SlowProblem
  seconds_to_sleep = '0.0 0.0 0.1 0.1 0.5 0.2 0.2 0.1 0.1 0.1'
  kernel_coverage_check = false
[]

[Executioner]
  type = Transient
  num_steps = 10

  [TimeStepper]
    type = SolutionTimeAdaptiveDT
    adapt_log = true
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
