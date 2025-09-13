# Coupled Cahn-Hilliard and Mechanics with improved string expressions
# 
# This example demonstrates:
# 1. Multiple energy expressions for coupled PDEs
# 2. Intermediate expression definitions (strain tensor)
# 3. Vector assembly using vec() operator
# 
# Energy functional:
# F[c,u] = ∫ [f_ch(c,∇c) + f_elastic(ε,c)] dx
# where:
#   f_ch = (c^2-1)^2 + κ/2|∇c|^2
#   f_elastic = λ/2(tr(ε))^2 + μ|ε|^2 + α*c*tr(ε)
#   ε = sym(∇u) with u assembled from components

[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 40
  ny = 40
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
  [disp_x]
    order = FIRST
    family = LAGRANGE
  []
  [disp_y]
    order = FIRST
    family = LAGRANGE
  []
[]

[VariationalProblem]
  [coupled_system]
    type = VariationalDerivativeAction
    energy_type = expression
    
    # Define intermediate expressions first
    expressions = 'u = vec(disp_x, disp_y)
                   strain = sym(grad(u))
                   tr_strain = trace(strain)
                   strain_norm2 = contract(strain, strain)'
    
    # Multiple energy expressions for each primary variable
    energy_expressions = 'c = W(c) + 0.5*kappa*dot(grad(c), grad(c)) + alpha*c*tr_strain
                          disp_x = 0.5*lambda*pow(tr_strain, 2) + mu*strain_norm2
                          disp_y = 0.5*lambda*pow(tr_strain, 2) + mu*strain_norm2'
    
    # Parameters for the energy expressions
    parameters = 'kappa=0.01 lambda=100 mu=75 alpha=10'
    
    # Primary variables
    variables = 'c disp_x disp_y'
    
    # Enable automatic differentiation
    use_automatic_differentiation = true
    
    # Enable variable splitting if needed for higher-order terms
    enable_splitting = true
    max_fe_order = 1
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
  # Mechanical fields start at zero (default)
[]

[BCs]
  # Fix bottom edge mechanically (no displacement)
  [bottom_x]
    type = DirichletBC
    variable = disp_x
    boundary = bottom
    value = 0
  []
  [bottom_y]
    type = DirichletBC
    variable = disp_y
    boundary = bottom
    value = 0
  []
  
  # Apply tensile load at top
  [top_y]
    type = FunctionDirichletBC
    variable = disp_y
    boundary = top
    function = '0.002*t'
  []
  
  # Fix horizontal displacement at center of top to prevent rigid body motion
  [top_center_x]
    type = DirichletBC
    variable = disp_x
    boundary = top
    value = 0
  []
[]

[Executioner]
  type = Transient
  solve_type = NEWTON
  
  # Use direct solver for robustness
  petsc_options_iname = '-pc_type -pc_factor_mat_solver_package'
  petsc_options_value = 'lu superlu_dist'
  
  # Tolerances
  l_tol = 1e-4
  l_max_its = 30
  nl_max_its = 20
  nl_rel_tol = 1e-8
  nl_abs_tol = 1e-10
  
  # Time stepping
  dt = 5e-4
  end_time = 0.1
  
  # Adaptive time stepping
  [TimeStepper]
    type = IterationAdaptiveDT
    dt = 5e-4
    growth_factor = 1.2
    cutback_factor = 0.5
    optimal_iterations = 10
  []
[]

[Outputs]
  exodus = true
  print_linear_residuals = false
  print_nonlinear_residuals = true
  
  [console]
    type = Console
    print_mesh_changed_info = false
  []
  
  [checkpoint]
    type = Checkpoint
    interval = 10
  []
[]

[Postprocessors]
  # Monitor concentration
  [c_avg]
    type = ElementAverageValue
    variable = c
  []
  [c_min]
    type = ElementExtremeValue
    variable = c
    value_type = min
  []
  [c_max]
    type = ElementExtremeValue
    variable = c
    value_type = max
  []
  
  # Monitor displacement
  [disp_y_max]
    type = NodalExtremeValue
    variable = disp_y
    boundary = top
  []
  
  # Monitor energy (if available)
  [total_energy]
    type = ElementIntegralVariablePostprocessor
    variable = c  # placeholder - would need actual energy density aux variable
  []
[]