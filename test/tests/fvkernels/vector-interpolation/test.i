a=1.1

[Mesh]
  [gen_mesh]
    type = GeneratedMeshGenerator
    dim = 1
    xmin = 0.1
    xmax = 1.1
    nx = 20
  []
[]

[GlobalParams]
  limiter = 'vanLeer'
[]

[ICs]
  [u]
    type = FunctionIC
    variable = u
    function = exact
  []
  [v]
    type = FunctionIC
    variable = v
    function = exact
  []
  [w]
    type = FunctionIC
    variable = w
    function = exact
  []
[]

[Variables]
  [u]
    type = MooseVariableFVReal
  []
  [v]
    type = MooseVariableFVReal
  []
  [w]
    type = MooseVariableFVReal
  []
[]

[FVKernels]
  [advection_u]
    type = FVLimitedVectorAdvection
    variable = u
    velocity = '${a} 0 0'
    boundaries_to_force = 'right'
    x_functor = 'u'
    y_functor = 'v'
    z_functor = 'w'
    component = 0
  []
  [body_u]
    type = FVBodyForce
    variable = u
    function = 'forcing'
  []
  [advection_v]
    type = FVLimitedVectorAdvection
    variable = v
    velocity = '${a} 0 0'
    boundaries_to_force = 'right'
    x_functor = 'u'
    y_functor = 'v'
    z_functor = 'w'
    component = 1
  []
  [body_v]
    type = FVBodyForce
    variable = v
    function = 'forcing'
  []
  [advection_w]
    type = FVLimitedVectorAdvection
    variable = w
    velocity = '${a} 0 0'
    boundaries_to_force = 'right'
    x_functor = 'u'
    y_functor = 'v'
    z_functor = 'w'
    component = 2
  []
  [body_w]
    type = FVBodyForce
    variable = w
    function = 'forcing'
  []
[]

[FVBCs]
  [left_u]
    type = FVFunctionNeumannBC
    boundary = 'left'
    function = 'advection'
    variable = u
  []
  [left_v]
    type = FVFunctionNeumannBC
    boundary = 'left'
    function = 'advection'
    variable = v
  []
  [left_w]
    type = FVFunctionNeumannBC
    boundary = 'left'
    function = 'advection'
    variable = w
  []
[]

[Functions]
  [exact]
    type = ParsedFunction
    expression = 'cos(x)'
  []
  [advection]
    type = ParsedFunction
    expression = '${a} * cos(x)'
  []
  [forcing]
    type = ParsedFunction
    expression = '-${a} * sin(x)'
  []
[]

[Executioner]
  type = Steady
  solve_type = 'NEWTON'
  petsc_options_iname = '-pc_type -pc_factor_shift_type'
  petsc_options_value = 'lu       NONZERO'
[]

[Outputs]
  exodus = true
[]
