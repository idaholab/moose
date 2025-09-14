[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 100
  xmin = -10
  xmax = 10
[]

[AutomaticWeakForm]
  type = AutomaticWeakFormAction
  energy_type = EXPRESSION
  energy_expression = '0.25*(c^2 - 1)^2 + 0.5*kappa*dot(grad(c), grad(c)) + 0.01*dot(grad(grad(c)), grad(grad(c)))'
  variables = 'c'
  parameters = 'kappa 0.01'
  enable_splitting = true
  max_fe_order = 1  # Forces splitting for 2nd derivatives
  use_automatic_differentiation = true
  verbose = true
  debug_print_expressions = true
[]

[ICs]
  [c_ic]
    type = FunctionIC
    variable = c
    function = 'tanh(x)'
  []
[]

[BCs]
  [left]
    type = DirichletBC
    variable = c
    boundary = left
    value = -1
  []
  [right]
    type = DirichletBC
    variable = c
    boundary = right
    value = 1
  []
[]

[Executioner]
  type = Steady
  solve_type = NEWTON
  petsc_options_iname = '-pc_type -ksp_gmres_restart -sub_pc_type -pc_asm_overlap'
  petsc_options_value = 'asm      31                  lu           1'
  l_tol = 1e-4
  l_max_its = 30
  nl_rel_tol = 1e-8
  nl_abs_tol = 1e-10
  nl_max_its = 20
[]

[Outputs]
  exodus = true
  print_linear_residuals = false
  print_nonlinear_residuals = true
[]