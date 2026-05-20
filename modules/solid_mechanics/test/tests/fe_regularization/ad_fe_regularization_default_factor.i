reg_kernel = ADFERegularization
regularization_type = huhu_lulu
forcing_scale = 0.5

[GlobalParams]
  symbol_names = 'c'
  symbol_values = '5'
[]

[Mesh]
  type = GeneratedMesh
  dim = 2
  elem_type = QUAD9
  xmin = -0.5
  xmax = 0.5
  ymin = -0.5
  ymax = 0.5
  nx = 4
  ny = 4
[]

[Variables]
  [u]
    order = SECOND
    family = LAGRANGE
  []
[]

[Kernels]
  # The second-derivative regularization is tested on C0 quadratic Lagrange elements. The
  # diffusion term keeps this test problem well posed while the regularization block
  # exercises the HuHu/LuLu residual and Jacobian contributions.
  [diffusion]
    type = ADDiffusion
    variable = u
  []
  [regularization]
    type = ${reg_kernel}
    variable = u
    regularization = ${regularization_type}
    coefficient = 1
  []
  [body_force]
    type = BodyForce
    variable = u
    function = forcing_func
  []
[]

[BCs]
  [boundary_value]
    type = FunctionPenaltyDirichletBC
    variable = u
    boundary = 'left right top bottom'
    function = u_func
    penalty = 1e8
  []
[]

[Functions]
  [u_func]
    type = ParsedFunction
    expression = 'exp(-c*(x^2+y^2))'
  []
  [forcing_func]
    type = ParsedFunction
    expression = '${forcing_scale} * 16*c^2*(c^2*(x^2+y^2)^2 - 4*c*(x^2+y^2) + 2)*exp(-c*(x^2+y^2))'
  []
[]

[Postprocessors]
  [l2_error]
    type = ElementL2Error
    variable = u
    function = u_func
  []
  [h1_error]
    type = ElementH1Error
    variable = u
    function = u_func
  []
[]

[Executioner]
  type = Steady
  solve_type = NEWTON
  nl_rel_tol = 1e-12
  nl_abs_tol = 1e-12
  l_tol = 1e-12
  petsc_options_iname = '-pc_type'
  petsc_options_value = 'lu'

  [Quadrature]
    type = GAUSS
    order = ELEVENTH
  []
[]

[Outputs]
  csv = false
[]
