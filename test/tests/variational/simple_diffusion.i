# Simple diffusion test to verify AutomaticWeakFormAction is working
# Energy: F[u] = ∫ (1/2)|∇u|^2 dx
# This should generate -∇²u = 0

[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 10
  ny = 10
[]

[Variables]
  [u]
    order = FIRST
    family = LAGRANGE
    initial_condition = 0.5
  []
[]

[AutomaticWeakForm]
  energy_type = expression
  energy_expression = '0.5*dot(grad(u), grad(u))'
  variables = 'u'
  use_automatic_differentiation = false
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
    value = 1
  []
[]

[Executioner]
  type = Steady
  solve_type = NEWTON
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
  l_tol = 1e-10
  nl_rel_tol = 1e-10
  nl_abs_tol = 1e-10
[]

[Outputs]
  exodus = true
[]