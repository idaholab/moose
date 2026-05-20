reg_kernel = FERegularization

[GlobalParams]
  symbol_names = 'c'
  symbol_values = '5'
[]

[Mesh]
  type = GeneratedMesh
  dim = 3
  elem_type = HEX27
  xmin = -0.5
  xmax = 0.5
  ymin = -0.5
  ymax = 0.5
  zmin = -0.5
  zmax = 0.5
  nx = 2
  ny = 2
  nz = 2
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
    type = Diffusion
    variable = u
  []
  [regularization]
    type = ${reg_kernel}
    variable = u
    regularization = huhu_lulu
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
    boundary = 'left right top bottom front back'
    function = u_func
    penalty = 1e8
  []
[]

[Functions]
  [u_func]
    type = ParsedFunction
    expression = 'exp(-c*(x^2+y^2+z^2))'
  []
  [forcing_func]
    type = ParsedFunction
    expression = '2.0/3.0 * 16*c^2*(c^2*(x^2+y^2+z^2)^2 - 5*c*(x^2+y^2+z^2) + 15.0/4.0)*exp(-c*(x^2+y^2+z^2))'
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
