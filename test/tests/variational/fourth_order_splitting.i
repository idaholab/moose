# Test automatic variable splitting for 4th-order Cahn-Hilliard equation
# Energy: F[c] = ∫ [W(c) + κ/2|∇c|² + λ/2|∇²c|²] dx
# This requires splitting since standard FE only provides up to 1st derivatives

[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 40
  xmin = -1
  xmax = 1
[]

[Variables]
  [c]
    order = FIRST  # Only first-order elements
    family = LAGRANGE
    [InitialCondition]
      type = FunctionIC
      function = '0.1*sin(3.14159*x)'
    []
  []
[]

[AutomaticWeakForm]
  energy_type = expression
  # Energy with 4th-order term requiring splitting
  # Using (∇²c)² which gives a 4th-order PDE
  energy_expression = '(c*c - 1.0)*(c*c - 1.0) + 0.5*kappa*dot(grad(c), grad(c)) + 0.5*lambda*dot(grad(grad(c)), grad(grad(c)))'
  variables = 'c'
  parameters = 'kappa 0.01 lambda 0.001'
  
  # Enable automatic splitting for higher-order derivatives
  enable_splitting = true
  max_fe_order = 1  # Forces splitting for 2nd derivatives
  
  # Enable verbose output to see the splitting process
  verbose = true
  debug_print_weak_form = true
  
  # Use automatic differentiation
  use_automatic_differentiation = true
[]

[BCs]
  [left]
    type = NeumannBC
    variable = c
    boundary = left
    value = 0
  []
  [right]
    type = NeumannBC
    variable = c
    boundary = right
    value = 0
  []
[]

[Executioner]
  type = Steady
  solve_type = NEWTON
  
  petsc_options_iname = '-pc_type -ksp_type'
  petsc_options_value = 'lu gmres'
  
  l_tol = 1e-10
  nl_rel_tol = 1e-8
  nl_abs_tol = 1e-10
  nl_max_its = 50
  
  automatic_scaling = true
[]

[Outputs]
  exodus = true
  perf_graph = true
  
  # Output the auxiliary variables (split variables)
  [console]
    type = Console
    show_aux = true
  []
[]