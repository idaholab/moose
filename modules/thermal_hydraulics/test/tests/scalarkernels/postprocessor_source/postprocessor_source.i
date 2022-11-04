# This input file tests PostprocessorSourceScalarKernel.
#
# The following initial value problem is modeled here:
#   du/dt = t,  u(0) = 0
# Using backward Euler time integration with dt=1, the solution values should
# be as follows:
#   u(0) = 0
#   u(1) = 1
#   u(2) = 3


[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 1
[]

[Variables]
  [u]
    family = SCALAR
    order = FIRST
  []
[]

[ICs]
  [ic_u]
    type = ScalarConstantIC
    variable = u
    value = 0
  []
[]

[ScalarKernels]
  [sk_time]
    type = ODETimeDerivative
    variable = u
  []
  [sk_source]
    type = PostprocessorSourceScalarKernel
    variable = u
    pp = pp_source
  []
[]

[Functions]
  [fn_source]
    type = ParsedFunction
    expression = 't'
  []
[]

[Postprocessors]
  [pp_source]
    type = FunctionValuePostprocessor
    function = fn_source
    execute_on = 'LINEAR NONLINEAR'
  []
[]

[Executioner]
  type = Transient
  scheme = implicit-euler
  dt = 1
  num_steps = 2
[]

[Outputs]
  csv = true
  show = 'u'
  execute_on = 'INITIAL TIMESTEP_END'
[]
