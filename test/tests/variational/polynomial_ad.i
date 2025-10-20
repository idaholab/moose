# Test polynomial energy with AD Jacobian
# Energy: F[u] = ∫ (u^4 - 2u^2 + 1) dx
# This should give δF/δu = 4u^3 - 4u

[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 20
  xmin = -1
  xmax = 1
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
    energy_expression = 'u*u*u*u - 2.0*u*u + 1.0'
    variables = 'u'
    use_automatic_differentiation = true
[]

[BCs]
  [left]
    type = DirichletBC
    variable = u
    boundary = left
    value = -1
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
  
  petsc_options_iname = '-pc_type'
  petsc_options_value = 'lu'
  
  l_tol = 1e-10
  nl_rel_tol = 1e-10
  nl_abs_tol = 1e-12
[]

[Outputs]
  exodus = true
[]