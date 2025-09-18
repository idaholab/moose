###########################################################
# This is a test of the ADScaledCoupledTimeDerivative kernel
# solving the following PDE system using mms:
# du/dt - div(grad(u)) = f_u
# v*du/dt - div(grad(v)) = f_v
# The manufactured solution for the variables u and v are
# u(t,x,y) = t^4*exp(x+y) and v(x,y) = sin(2*pi*x)*cos(2*pi*y)
###########################################################

[Mesh]
  type = GeneratedMesh
  nx = 10
  ny = 10
  dim = 2
[]

[Variables]
  [u]
  []
  [v]
  []
[]

[Kernels]
  [time_u]
    type = ADTimeDerivative
    variable = u
  []
  [diff_u]
    type = ADDiffusion
    variable = u
  []
  [fn_u]
    type = ADBodyForce
    variable = u
    function = force_u
  []
  [time_v]
    type = ADScaledCoupledTimeDerivative
    variable = v
    v = u
    mat_prop = mat_prop
  []
  [diff_v]
    type = ADDiffusion
    variable = v
  []
  [fn_v]
    type = ADBodyForce
    variable = v
    function = force_v
  []
[]

[Materials]
  [mat_prop]
    type = ADParsedMaterial
    property_name = mat_prop
    coupled_variables = 'v'
    expression = 'v'
  []
[]

[BCs]
  [allv]
    type = FunctionDirichletBC
    variable = v
    boundary = 'left right bottom top'
    function = v_exact
  []
  [allu]
    type = FunctionDirichletBC
    variable = u
    boundary = 'left right bottom top'
    function = u_exact
  []
[]

[Functions]

  ### u manufactured terms ###

  [u_exact]
    type = ParsedFunction
    expression = 't^4*exp(x+y)'
  []
  [du_exact]
    type = ParsedFunction
    expression = '4*t^3*exp(x+y)'
  []
  [laplacian_u]
    type = ParsedFunction
    expression = '2*t^4*exp(x+y)'
  []
  [force_u]
    type = ParsedFunction
    symbol_names = 'du_exact laplacian_u'
    symbol_values = 'du_exact laplacian_u'
    expression = 'du_exact - laplacian_u'
  []

  ### v manufactured terms ###

  [v_exact]
    type = ParsedFunction
    expression = 'sin(2*pi*x)*cos(2*pi*y)'
  []
  [laplacian_v]
    type = ParsedFunction
    expression = '-8*pi^2*sin(2*x*pi)*cos(2*y*pi)'
  []
  [force_v]
    type = ParsedFunction
    symbol_names = 'v_exact du_exact laplacian_v'
    symbol_values = 'v_exact du_exact laplacian_v'
    expression = 'v_exact*du_exact - laplacian_v'
  []
[]

[Postprocessors]
  [error_v]
    type = ElementL2Error
    function = v_exact
    variable = v
  []
  [error_u]
    type = ElementL2Error
    function = u_exact
    variable = u
  []
[]

[Executioner]
  type = Transient
  dt = 0.1
  end_time = 1
  scheme = BDF2
  solve_type = 'Newton'
  petsc_options_iname = '-pc_type'
  petsc_options_value = 'lu'
[]

[Outputs]
  exodus = true
[]
