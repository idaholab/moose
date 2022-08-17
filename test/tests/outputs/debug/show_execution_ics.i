[Mesh]
  type = GeneratedMesh
  dim = 2
  xmin = 0
  xmax = 1
  ymin = 0
  ymax = 1
  nx = 2
  ny = 2
[]

[Debug]
  show_execution_order = ALWAYS
[]

[AuxVariables]
  [a]
  []

  [b]
  []
[]

[Variables]
  [u]
  []

  [v]
  []
[]

# From dependency ics test
[ICs]
  [u_ic]
    type = ConstantIC
    variable = u
    value = -1
  []

  [v_ic]
    type = MTICSum
    variable = v
    var1 = u
    var2 = a
  []

  [a_ic]
    type = ConstantIC
    variable = a
    value = 10
  []

  [b_ic]
    type = MTICMult
    variable = b
    var1 = v
    factor = 2
  []
[]

[AuxKernels]
  [a_ak]
    type = ConstantAux
    variable = a
    value = 256
  []

  [b_ak]
    type = ConstantAux
    variable = b
    value = 42
  []
[]

[Kernels]
  [diff_u]
    type = Diffusion
    variable = u
  []

  [diff_v]
    type = Diffusion
    variable = v
  []
[]

# From depend on uo test
[AuxVariables]
  [ghost]
    family = MONOMIAL
    order = CONSTANT
  []
[]

[ICs]
  [ghost_ic]
    type = ElementUOIC
    variable = ghost
    element_user_object = ghost_uo
    field_name = "ghosted"
    field_type = long
  []
[]

[UserObjects]
  [ghost_uo]
    type = ElemSideNeighborLayersTester
    execute_on = initial
    element_side_neighbor_layers = 1
  []
[]

# From postprocessor interface ICs test
[Functions]
  # The integral of this function is 2*3 + 3*6 + 5*2 = 34
  [test_fn]
    type = PiecewiseConstant
    axis = x
    x = '0 2 5'
    y = '3 6 2'
  []
[]

[Postprocessors]
  [integral_pp]
    type = FunctionElementIntegral
    function = test_fn
    execute_on = 'INITIAL'
  []
  [pp2]
    type = FunctionValuePostprocessor
    function = 6
    execute_on = 'INITIAL'
  []
[]

[AuxVariables]
  [test_var]
    order = CONSTANT
    family = MONOMIAL
  []
[]

[ICs]
  [test_var_ic]
    type = PostprocessorIC
    variable = test_var
    pp1 = integral_pp
  []
[]

# From integral preserving test
[AuxVariables]
  [power]
    family = MONOMIAL
    order = CONSTANT
  []
[]

[ICs]
  [power]
    type = IntegralPreservingFunctionIC
    variable = power
    magnitude = 550.0
    function = 'sin(pi * z / 1.9)'
    integral = vol
  []
[]

[Postprocessors]
  [vol]
    type = FunctionElementIntegral
    function = 'sin(pi * x / 1.9)'
    execute_on = 'initial'
  []
[]

[BCs]
  [left_u]
    type = DirichletBC
    variable = u
    boundary = left
    value = 0
  []

  [right_u]
    type = DirichletBC
    variable = u
    boundary = right
    value = 1
  []


  [left_v]
    type = DirichletBC
    variable = v
    boundary = left
    value = 2
  []

  [right_v]
    type = DirichletBC
    variable = v
    boundary = right
    value = 1
  []
[]

[Executioner]
  type = Steady

  solve_type = 'NEWTON'
  nl_rel_tol = 1e-10
[]
