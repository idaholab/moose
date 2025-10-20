# Test if grad(grad(c)) shape is handled correctly
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
  # Energy that produces grad(grad(c)) directly
  # Using trace to get a scalar from the Hessian
  energy_expression = 'trace(grad(grad(c)))'
  variables = 'c'
  
  verbose = true
  debug_print_expressions = true
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