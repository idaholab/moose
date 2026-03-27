# Sub-app for TwoMultiAppNewtonIterationUserObject test.
#
# Solves the scalar ODE:  du/dt = param_pp
#
# param_pp is a Receiver postprocessor set by the parent each Newton iteration.
# The ODE solution accumulates: u(t_n) = u(t_{n-1}) + param_pp * dt.
#
# With the parent's Newton iteration choosing param_pp = 2*t - dt (for dt=1):
#   u(1) = 1,  u(2) = 4,  u(3) = 9   =>  u(t) = t^2
#
# This exactly matches the parent's target function t^2, verifiable from
# the output CSV (output_pp = u).

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
  # du/dt term
  [time_deriv]
    type = ODETimeDerivative
    variable = u
  []
  # Source term: residual contribution = -param_pp
  # Combined with ODETimeDerivative: du/dt - param_pp = 0  =>  du/dt = param_pp
  [source]
    type = ParsedODEKernel
    variable = u
    expression = '-param_pp'
    postprocessors = param_pp
  []
[]

[Postprocessors]
  # Receives the rate parameter from the parent app before each solve.
  [param_pp]
    type = Receiver
    default = 0.0
  []

  # Output: the scalar ODE solution u(t), read as a postprocessor.
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
