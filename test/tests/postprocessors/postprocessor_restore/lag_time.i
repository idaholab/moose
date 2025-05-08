[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 1
  []
[]

[Variables]
  [u]
  []
[]

[Kernels]
  [dt]
    type = TimeDerivative
    variable = u
  []
  [source]
    type = BodyForce
    variable = u
  []
[]

[Postprocessors]
  [time]
    type = TimePostprocessor
    execute_on = 'nonlinear'
  []
  [time_lag]
    type = ParsedPostprocessor
    expression = 'time'
    pp_names = 'time'
    execute_on = 'timestep_begin'
  []
[]

[Executioner]
  type = Transient
  end_time = 5
[]

[Problem]
  type = FailingProblem
  fail_steps = 5
[]

[Outputs]
  csv = true
[]

