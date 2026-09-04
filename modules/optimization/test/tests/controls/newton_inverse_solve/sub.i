# Nonlinear forward model for the NewtonInversionControl test.
#
# Solves the scalar ODE:  du/dt = param_pp^3,  u(0) = 0
# After one unit step (dt = 1) the output is u = param_pp^3, so the per-step map
# y(p) = p^3 is strongly nonlinear (an inflection at p = 0).
# param_pp is a Receiver set by the parent each fixed-point iteration.

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
    expression = '-param_pp^3'
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
