# Integration test for automatic splitting on a fourth-order energy

[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 4
  xmin = -1
  xmax = 1
[]

[Variables]
  [c]
    family = LAGRANGE
    order = FIRST
    initial_condition = 0.0
  []
[]

[AutomaticWeakForm]
  energy_type = expression
  energy_expression = '0.25*(c^2 - 1)^2 + 0.5*kappa*dot(grad(c), grad(c)) + 0.5*lambda*dot(grad(grad(c)), grad(grad(c)))'
  variables = 'c'
  parameters = 'kappa 0.01 lambda 0.001'
  enable_splitting = true
  max_fe_order = 1
  verbose = true
  debug_print_weak_form = true
  use_automatic_differentiation = true
[]

[Executioner]
  type = Steady
  solve_type = NEWTON
[]

[Problem]
  solve = false
[]

