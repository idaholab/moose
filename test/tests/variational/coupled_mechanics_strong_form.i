# Coupled phase-field mechanics using strong form syntax
#
# Strong form equations:
#   ∂c/∂t = -∇·(M∇μ)           (conserved dynamics)
#   μ = dW/dc - κ∇²c + α·tr(ε)  (chemical potential with elastic coupling)
#   0 = ∇·σ                     (mechanical equilibrium)
#   σ = λ·tr(ε)I + 2μ·ε + α·c·I (stress with concentration coupling)
#
# This demonstrates:
# 1. Multiple coupled PDEs with mixed time-dependent and steady equations
# 2. Vector assembly from components using vec()
# 3. Tensor operations with intermediate expressions
# 4. Automatic weak form derivation and Jacobian computation

[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 30
  ny = 30
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
  [mu]
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

[AutomaticWeakForm]
  [coupled_pf_mechanics]
    type = AutomaticWeakFormAction
    energy_type = expression
    
    # Intermediate expressions for clarity and reuse
    expressions = 'u = vec(disp_x, disp_y);
                   grad_u = grad(u);
                   strain = sym(grad_u);
                   tr_strain = trace(strain);
                   strain_energy = 0.5*lambda*pow(tr_strain, 2) + mu*contract(strain, strain);
                   coupling_energy = alpha*c*tr_strain;
                   stress = lambda*tr_strain*I + 2*mu*strain + alpha*c*I;
                   M = 1.0;
                   dW_dc = 4*c*(c^2 - 1)'
    
    # Strong form equations
    # Time derivatives: var_t = RHS
    # Steady state: var = RHS (interpreted as 0 = RHS - var)
    strong_forms = 'c_t = div(M*grad(mu));
                    mu = dW_dc - kappa*laplacian(c) + alpha*tr_strain;
                    disp_x = -div_x(stress);
                    disp_y = -div_y(stress)'
    
    # Material and coupling parameters
    parameters = 'kappa=0.01 lambda=100 mu=75 alpha=10'
    
    # Primary variables
    variables = 'c mu disp_x disp_y'
    
    # Enable automatic differentiation
    use_automatic_differentiation = true
    
    # Variable splitting for higher-order terms if needed
    enable_splitting = false
  []
[]

[ICs]
  [c_IC]
    type = RandomIC
    variable = c
    min = -0.05
    max = 0.05
    seed = 12345
  []
  [mu_IC]
    type = ConstantIC
    variable = mu
    value = 0
  []
  # Displacement fields start at zero (default)
[]

[BCs]
  # Mechanical boundary conditions
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
  
  # Applied displacement at top
  [top_y]
    type = FunctionDirichletBC
    variable = disp_y
    boundary = top
    function = '0.001*t'
  []
  
  # Prevent rigid body motion
  [top_center_x]
    type = DirichletBC
    variable = disp_x
    boundary = 'top'
    value = 0
  []
  
  # Natural BCs for c and mu (no flux)
[]

[Executioner]
  type = Transient
  solve_type = NEWTON
  
  # Direct solver for coupled system
  petsc_options_iname = '-pc_type -pc_factor_mat_solver_package'
  petsc_options_value = 'lu superlu_dist'
  
  # Tolerances
  l_tol = 1e-4
  l_max_its = 40
  nl_max_its = 25
  nl_rel_tol = 1e-8
  nl_abs_tol = 1e-10
  
  # Time stepping
  dt = 2e-4
  end_time = 0.05
  
  [TimeStepper]
    type = IterationAdaptiveDT
    dt = 2e-4
    growth_factor = 1.1
    cutback_factor = 0.5
    optimal_iterations = 12
  []
[]

[Outputs]
  exodus = true
  print_linear_residuals = false
  
  [console]
    type = Console
    print_mesh_changed_info = false
    max_rows = 20
  []
[]

[Postprocessors]
  # Phase field monitoring
  [c_avg]
    type = ElementAverageValue
    variable = c
    execute_on = 'initial timestep_end'
  []
  [c_range]
    type = ElementExtremeValue
    variable = c
    value_type = range
    execute_on = 'timestep_end'
  []
  
  # Chemical potential
  [mu_avg]
    type = ElementAverageValue
    variable = mu
    execute_on = 'timestep_end'
  []
  
  # Mechanical monitoring
  [disp_y_max]
    type = NodalExtremeValue
    variable = disp_y
    boundary = top
    execute_on = 'timestep_end'
  []
  
  [strain_energy]
    type = ElementIntegralVariablePostprocessor
    variable = c  # Placeholder - would need aux variable for actual strain energy
    execute_on = 'timestep_end'
  []
[]

[AuxVariables]
  # Could add auxiliary variables for stress, strain components if needed
  [von_mises_stress]
    order = CONSTANT
    family = MONOMIAL
  []
[]

[AuxKernels]
  # Could compute von Mises stress or other derived quantities
[]