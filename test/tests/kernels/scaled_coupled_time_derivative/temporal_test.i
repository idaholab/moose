###########################################################
# This is a temporal convergence test of the ADScaledCoupledTimeDerivative kernel
# solving the following PDE system using mms:
# du/dt = f_u
# v*du/dt - div(grad(v)) = f_v
# The manufactured solution for the variables u and v are
# u(t) = t*exp(t) and v(x,y) = x*y
###########################################################

[Mesh]
  type = GeneratedMesh
  nx = 5
  ny = 5
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
  [fn_u]
    type = ADBodyForce
    variable = u
    function = du_exact
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
    function = force
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
  [u_exact]
    type = ParsedFunction
    expression = 't*exp(t)'
  []
  [du_exact]
    type = ParsedFunction
    expression = '(1+t)*exp(t)'
  []
  [v_exact]
    type = ParsedFunction
    expression = 'x*y' # Lagrange elements are exact to negate spatial error
  []
  [laplacian_v]
    type = ParsedFunction
    expression = '0'
  []
  [force]
    type = ParsedFunction
    symbol_names = 'du_exact v_exact laplacian_v'
    symbol_values = 'du_exact v_exact laplacian_v'
    expression = 'du_exact*v_exact - laplacian_v'
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
  [h]
    type = AverageElementSize
  []
[]

[Executioner]
  type = Transient
  dt = 0.1
  end_time = 1
  solve_type = 'Newton'
  petsc_options_iname = '-pc_type'
  petsc_options_value = 'lu'
[]

[Outputs]
  exodus = true
  csv = true
[]
