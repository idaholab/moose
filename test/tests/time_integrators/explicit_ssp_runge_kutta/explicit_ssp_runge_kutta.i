# This test solves the following IVP:
#   du/dt = f(u(t), t),   u(0) = 1
#   f(u(t), t) = -u(t) + t^3 + 3t^2
# The exact solution is the following:
#   u(t) = exp(-t) + t^3

[Mesh]
  [./mesh]
    type = GeneratedMeshGenerator
    dim = 1
    nx = 1
  [../]
[]

[Variables]
  [./u]
    family = SCALAR
    order = FIRST
    initial_condition = 1
  [../]
[]

[ScalarKernels]
  [./time_derivative]
    type = ODETimeDerivative
    variable = u
  [../]
  [./source_part1]
    type = ParsedODEKernel
    variable = u
    expression = 'u'
  [../]
  [./source_part2]
    type = PostprocessorSinkScalarKernel
    variable = u
    postprocessor = sink_pp
  [../]
[]

[Functions]
  [./sink_fn]
    type = ParsedFunction
    expression = '-t^3 - 3*t^2'
  [../]
[]

[Postprocessors]
  [./sink_pp]
    type = FunctionValuePostprocessor
    function = sink_fn
    execute_on = 'LINEAR NONLINEAR'
  [../]
  [./l2_err]
    type = ScalarL2Error
    variable = u
    function = ${fparse exp(-0.5) + 0.5^3}
  [../]
[]

[Executioner]
  type = Transient

  [./TimeIntegrator]
    type = ExplicitSSPRungeKutta
    order = 1
  [../]

  end_time = 0.5
  dt = 0.1
[]

[Outputs]
  file_base = 'first_order'
  [./csv]
    type = CSV
    show = 'u'
    execute_on = 'FINAL'
  [../]
[]
