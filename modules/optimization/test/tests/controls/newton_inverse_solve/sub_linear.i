# Linear forward model for the NewtonInversionControl parity test.
#
# Solves the scalar ODE:  du/dt = param_pp,  u(0) = 0
# Accumulates: u(t_n) = u(t_{n-1}) + param_pp * dt (per-step map y(p) linear in p).

[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 1
[]

[Variables]
  [u]
    family = SCALAR
    order = FIRST
    initial_condition = 0.0
  []
[]

[ScalarKernels]
  [time_deriv]
    type = ODETimeDerivative
    variable = u
  []
  [source]
    type = ParsedODEKernel
    variable = u
    expression = '-param_pp'
    postprocessors = param_pp
  []
[]

[Postprocessors]
  [param_pp]
    type = Receiver
    default = 0.0
  []
  [output_pp]
    type = ScalarVariable
    variable = u
    execute_on = 'INITIAL TIMESTEP_END'
  []
[]

[Executioner]
  type = Transient
  num_steps = 5
  dt = 1
  solve_type = NEWTON
[]

[Outputs]
  csv = false
[]
