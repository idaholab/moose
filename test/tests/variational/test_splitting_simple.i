# Simple test to verify variable splitting infrastructure
# This uses a simple case where we artificially lower max_fe_order to force splitting

[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 20
  xmin = 0
  xmax = 1
[]

[Variables]
  [u]
    order = FIRST
    family = LAGRANGE
    initial_condition = 0
  []
[]

[AutomaticWeakForm]
  energy_type = expression
  # Simple diffusion energy that normally doesn't need splitting
  energy_expression = '0.5*dot(grad(u), grad(u))'
  variables = 'u'
  
  # Force splitting by setting max_fe_order to 0
  # This will make the system think gradients aren't available
  enable_splitting = true
  max_fe_order = 0  # Artificially low to force splitting
  
  # Verbose output to debug
  verbose = true
  debug_print_expressions = true
  debug_print_weak_form = true
  
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
    value = 1
  []
[]

[Executioner]
  type = Steady
  solve_type = NEWTON
  
  petsc_options_iname = '-pc_type'
  petsc_options_value = 'lu'
  
  nl_rel_tol = 1e-10
  nl_abs_tol = 1e-12
[]

[Outputs]
  exodus = true
[]