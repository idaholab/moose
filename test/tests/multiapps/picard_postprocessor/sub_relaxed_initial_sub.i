[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 1
[]

[Variables]
  [y]
    family = SCALAR
    order = FIRST
    initial_condition = 1.0
  []
[]

[ScalarKernels]
  [ode]
    type = ParsedODEKernel
    variable = y
    expression = 'y - 1 - 0.25 * x_recv'
    postprocessors = 'x_recv'
  []
[]

[Postprocessors]
  [x_recv]
    type = Receiver
    default = 1.0
    execute_on = 'initial timestep_begin'
  []
  [y_out]
    type = ScalarVariable
    variable = y
    execute_on = 'initial timestep_end'
  []
[]

[Executioner]
  type = Steady
  solve_type = NEWTON
  nl_abs_tol = 1e-12
[]

[Outputs]
  csv = true
[]
