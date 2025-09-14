# Cahn-Hilliard equation using string-based energy functional
# Energy: F[c] = ∫ [(c^2-1)^2 + κ/2|∇c|^2] dx
# Chemical potential: μ = δF/δc = 4c(c^2-1) - κ∇²c
# Evolution: ∂c/∂t = ∇·(M∇μ)

[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 50
  ny = 50
  xmin = 0
  xmax = 1
  ymin = 0
  ymax = 1
[]

[Variables]
  [c]
    order = FIRST
    family = LAGRANGE
  []
[]

[AutomaticWeakForm]
  [cahn_hilliard]
    # Use expression type with string parsing
    energy_type = expression
    
    # Energy functional as a string expression
    # W(c) is shorthand for double-well (c^2-1)^2
    energy_expression = 'W(c) + 0.5*kappa*dot(grad(c), grad(c))'
    
    # Parameters used in the expression
    parameters = 'kappa 0.01'
    
    # Primary variable
    variables = 'c'
    
    # Enable automatic differentiation for Jacobian
    use_automatic_differentiation = true
  []
[]

[ICs]
  [c_IC]
    type = RandomIC
    variable = c
    min = -0.1
    max = 0.1
    seed = 12345
  []
[]

[BCs]
  # Natural boundary conditions (no flux)
[]

[Executioner]
  type = Transient
  solve_type = NEWTON
  
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
  
  l_tol = 1e-4
  l_max_its = 30
  nl_max_its = 20
  nl_rel_tol = 1e-9
  nl_abs_tol = 1e-11
  
  dt = 1e-4
  end_time = 0.1
[]

[Outputs]
  exodus = true
  print_linear_residuals = false
  print_nonlinear_residuals = false
[]