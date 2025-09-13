# Cahn-Hilliard equation using strong form syntax
#
# Strong form equations:
#   ∂c/∂t = -∇·(M∇μ)  (conserved dynamics)
#   μ = dW/dc - κ∇²c   (chemical potential)
#
# This example demonstrates:
# 1. Strong form equation specification with time derivatives
# 2. Automatic separation of time derivative terms
# 3. Semicolon-separated intermediate expressions
# 4. Generic expression evaluation kernels

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
  [mu]
    order = FIRST
    family = LAGRANGE
  []
[]

[AutomaticWeakForm]
  [cahn_hilliard_system]
    energy_type = expression

    # Intermediate expressions (semicolon-separated)
    expressions = 'M = 1.0;
                   dW_dc = 4*c*(c^2 - 1);
                   kappa_term = kappa*laplacian(c)'

    # Strong form equations (semicolon-separated)
    # Format: variable_t = expression for time derivatives
    #         variable = expression for steady equations
    strong_forms = 'c_t = div(M*grad(mu));
                    mu = dW_dc - kappa_term'

    # Parameters
    parameters = 'kappa=0.01'

    # Primary variables
    variables = 'c mu'

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
  [mu_IC]
    type = FunctionIC
    variable = mu
    function = 0
  []
[]

[BCs]
  # Natural boundary conditions (no flux) for c
  # Natural boundary conditions for mu
[]

[Executioner]
  type = Transient
  solve_type = NEWTON

  # Preconditioner for saddle-point problem
  petsc_options_iname = '-pc_type -pc_factor_mat_solver_package'
  petsc_options_value = 'lu superlu_dist'

  # Alternative: Use fieldsplit for better performance
  # petsc_options_iname = '-pc_type -pc_fieldsplit_type -pc_fieldsplit_detect_saddle_point'
  # petsc_options_value = 'fieldsplit multiplicative true'

  # Tolerances
  l_tol = 1e-4
  l_max_its = 30
  nl_max_its = 20
  nl_rel_tol = 1e-9
  nl_abs_tol = 1e-11

  # Time stepping
  dt = 1e-4
  end_time = 0.05

  [TimeStepper]
    type = IterationAdaptiveDT
    dt = 1e-4
    growth_factor = 1.2
    cutback_factor = 0.5
    optimal_iterations = 8
  []
[]

[Outputs]
  exodus = true
  print_linear_residuals = false

  [console]
    type = Console
    print_mesh_changed_info = false
  []
[]

[Postprocessors]
  [c_avg]
    type = ElementAverageValue
    variable = c
    execute_on = 'initial timestep_end'
  []
  [c_integral]
    type = ElementIntegralVariablePostprocessor
    variable = c
    execute_on = 'initial timestep_end'
  []
  [mu_max]
    type = ElementExtremeValue
    variable = mu
    value_type = max
    execute_on = 'timestep_end'
  []
  [mu_min]
    type = ElementExtremeValue
    variable = mu
    value_type = min
    execute_on = 'timestep_end'
  []
[]
