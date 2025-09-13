# Coupled Cahn-Hilliard and Mechanics using string expressions
# 
# Energy functional:
# F[c,u] = ∫ [f_ch(c,∇c) + f_elastic(ε,c)] dx
# where:
#   f_ch = (c^2-1)^2 + κ/2|∇c|^2
#   f_elastic = λ/2(tr(ε))^2 + μ|ε|^2 + α*c*tr(ε)
#   ε = sym(∇u)

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
  # Chemical free energy
  [chemical_energy]
    type = VariationalDerivativeAction
    energy_type = expression
    energy_expression = 'W(c) + 0.5*kappa*dot(grad(c), grad(c))'
    parameters = 'kappa=0.01'
    variables = 'c'
    coupled_variables = 'disp_x disp_y'
  []
  
  # Elastic energy with concentration coupling
  [elastic_energy]
    type = VariationalDerivativeAction
    energy_type = expression
    
    # Define strain: ε = sym(∇u)
    # tr(ε) is the trace (volumetric strain)
    # |ε|^2 is the Frobenius norm squared (contract(ε,ε))
    energy_expression = '0.5*lambda*pow(trace(strain),2) + mu*contract(strain,strain) + alpha*c*trace(strain)'
    
    parameters = 'lambda=100 mu=75 alpha=10'
    variables = 'disp_x disp_y'
    coupled_variables = 'c'
    
    # Note: 'strain' would need to be defined as sym(grad(u)) in the actual implementation
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
[]

[BCs]
  # Fix bottom edge mechanically
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
  
  # Apply tension at top
  [top_y]
    type = FunctionDirichletBC
    variable = disp_y
    boundary = top
    function = '0.001*t'
  []
[]

[Executioner]
  type = Transient
  solve_type = NEWTON
  
  petsc_options_iname = '-pc_type -pc_factor_mat_solver_package'
  petsc_options_value = 'lu superlu_dist'
  
  l_tol = 1e-4
  l_max_its = 30
  nl_max_its = 20
  nl_rel_tol = 1e-8
  nl_abs_tol = 1e-10
  
  dt = 1e-3
  end_time = 0.1
[]

[Outputs]
  exodus = true
  print_linear_residuals = false
[]