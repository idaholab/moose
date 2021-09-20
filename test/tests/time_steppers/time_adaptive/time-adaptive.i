[Mesh]
  type = GeneratedMesh
  dim = 2
[]

[GlobalParams]
[]

[Variables]
  [u]
    order = FIRST
    family = LAGRANGE
  []
[]

[Kernels]
  [u]
    type = SlowKernel
    variable = u
    step_delay = '0.0 0.0 0.1 0.1 0.5 0.2 0.2 0.1 0.1 0.1'
  []
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
