# Cahn-Hilliard equation with automatic differentiation for Jacobian
# Energy: F[c] = ∫ [(c^2-1)^2 + κ/2|∇c|^2] dx
# Chemical potential: μ = δF/δc = 4c(c^2-1) - κ∇²c

[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 20
  ny = 20
  xmin = 0
  xmax = 1
  ymin = 0
  ymax = 1
[]

[Variables]
  [c]
    order = FIRST
    family = LAGRANGE
    [InitialCondition]
      type = RandomIC
      min = -0.1
      max = 0.1
      seed = 12345
    []
  []
[]

[AutomaticWeakForm]
  [cahn_hilliard]
    energy_type = expression
    # Explicit double-well energy with gradient penalty
    energy_expression = '((c*c - 1.0)*(c*c - 1.0)) + 0.5*0.01*dot(grad(c), grad(c))'
    variables = 'c'
    use_automatic_differentiation = true  # Enable AD for Jacobian
  []
[]

[BCs]
  # Natural boundary conditions (no flux)
[]

[Executioner]
  type = Transient
  solve_type = NEWTON
  
  # Time stepping
  dt = 0.001
  end_time = 0.1
  
  # Solver settings
  petsc_options_iname = '-pc_type -pc_hypre_type -ksp_gmres_restart'
  petsc_options_value = 'hypre boomeramg 100'
  
  # Tolerances
  l_tol = 1e-5
  l_max_its = 100
  nl_rel_tol = 1e-8
  nl_abs_tol = 1e-10
  nl_max_its = 10
[]

[Outputs]
  exodus = true
  perf_graph = true
  print_linear_residuals = false
[]