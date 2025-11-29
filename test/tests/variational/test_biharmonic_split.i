# Simple biharmonic test with splitting
# Energy: F = 0.5 * |∇²u|²

[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 20
  xmin = 0
  xmax = 1
[]

[AutomaticWeakForm]
  energy_type = EXPRESSION
  energy_expression = '0.5 * dot(grad(grad(u)), grad(grad(u)))'
  variables = 'u'
  enable_splitting = true
  max_fe_order = 1  # Force splitting
  use_automatic_differentiation = true
[]

[BCs]
  [left]
    type = DirichletBC
    variable = u
    boundary = left
    value = 0
  []
  [right]
    type = DirichletBC
    variable = u
    boundary = right
    value = 0
  []
[]

[Executioner]
  type = Steady
  solve_type = NEWTON
  petsc_options_iname = '-pc_type'
  petsc_options_value = 'lu'
  nl_rel_tol = 1e-8
  nl_abs_tol = 1e-10
[]

[Outputs]
  exodus = false
  console = true
[]