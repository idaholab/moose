# Minimal test for 4th-order term differentiation bug
[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 10
  xmin = 0
  xmax = 1
[]

[Variables]
  [c]
    order = FIRST
    family = LAGRANGE
  []
[]

[AutomaticWeakForm]
  energy_type = expression
  # Just the 4th-order term to isolate the issue
  energy_expression = '0.5*dot(grad(grad(c)), grad(grad(c)))'
  variables = 'c'
  
  verbose = true
  debug_print_expressions = true
  debug_print_derivatives = true
  debug_print_weak_form = true
  
  use_automatic_differentiation = true
[]

[Executioner]
  type = Steady
  solve_type = NEWTON
[]

[Outputs]
  exodus = true
[]