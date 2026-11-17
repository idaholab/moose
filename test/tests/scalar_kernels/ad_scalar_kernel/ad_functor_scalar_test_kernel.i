# Solves du/dt = (2 * t + x) + 1 + 2 + w and dw/dt = 1 with implicit Euler, where the right-hand
# sides are sums of functors that an ADScalarKernel evaluates at points given in the input:
# - the function 2 * t + x at x = 1,
# - a functor material with value 1,
# - a postprocessor with value 2, and
# - the scalar variable w.
# The exact solution at the end of each step n (dt = 1) is w_n = n and
# u_n = 3 * n * (n + 1) / 2 + 4 * n, so u (postprocessor u_value) is 7, 17, and 30 at steps 1, 2,
# and 3.

[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 1
[]

[Variables]
  [u]
    family = SCALAR
    order = FIRST
    initial_condition = 0
  []
  [w]
    family = SCALAR
    order = FIRST
    initial_condition = 0
  []
[]

[Functions]
  [ramp]
    type = ParsedFunction
    expression = '2 * t + x'
  []
[]

[FunctorMaterials]
  [fmat]
    type = ADGenericConstantFunctorMaterial
    prop_names = prop
    prop_values = 1
  []
[]

[ScalarKernels]
  [time]
    type = ADScalarTimeDerivative
    variable = u
  []
  [ramp_source]
    type = ADFunctorScalarTestKernel
    variable = u
    functor = ramp
    point = '1 0 0'
  []
  [constant_source]
    type = ADFunctorScalarTestKernel
    variable = u
    functor = prop
  []
  [pp_source]
    type = ADFunctorScalarTestKernel
    variable = u
    functor = source_pp
  []
  [scalar_source]
    type = ADFunctorScalarTestKernel
    variable = u
    functor = w
  []

  [time_w]
    type = ADScalarTimeDerivative
    variable = w
  []
  [w_source]
    type = ADFunctorScalarTestKernel
    variable = w
    functor = prop
  []
[]

[Postprocessors]
  [source_pp]
    type = ConstantPostprocessor
    value = 2
  []
  [u_value]
    type = ScalarVariable
    variable = u
    execute_on = 'initial timestep_end'
  []
[]

[Executioner]
  type = Transient
  scheme = implicit-euler
  dt = 1
  num_steps = 3
  solve_type = NEWTON
  nl_abs_tol = 1e-12
[]

[Outputs]
  csv = true
  show = 'u_value'
[]
