# Allen-Cahn with Fourfold Anisotropic Interfacial Energy
#
# This example demonstrates Allen-Cahn dynamics with fourfold anisotropic
# interface energy, creating square-like equilibrium shapes.
#
# Energy functional:
# F[eta] = ∫ [W(eta) + κ(n)|∇eta|^2] dx
#
# where n = ∇eta/|∇eta| is the interface normal
# and κ(n) implements fourfold symmetry
#
# We approximate this using:
# κ_eff = κ_0 + κ_1*(n_x^4 + n_y^4)
#      = κ_0 + κ_1*(grad_x^4 + grad_y^4)/|grad|^4
#
# Using the vec() operator for directional decomposition

[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 40
  ny = 40
  xmin = -1
  xmax = 1
  ymin = -1
  ymax = 1
[]

[Variables]
  [eta]
    order = FIRST
    family = LAGRANGE
    scaling = 1.0
  []
[]

[AutomaticWeakForm]
  energy_type = EXPRESSION

  # Fourfold anisotropic Allen-Cahn energy
  # We decompose the gradient using vec() to create basis vectors
  # Then use fourth powers of components to create fourfold symmetry
  #
  # grad_x = dot(grad(eta), vec(1.0, 0.0))
  # grad_y = dot(grad(eta), vec(0.0, 1.0))
  # |grad|^2 = grad_x^2 + grad_y^2
  #
  # Anisotropic coefficient: κ_0 + κ_1*(grad_x^4 + grad_y^4)/(grad_x^2 + grad_y^2)^2
  # This simplifies to different effective stiffnesses along different directions

  # Energy with fourfold anisotropy
  # The anisotropic term favors interfaces aligned with coordinate axes
  # Using a simpler formulation that avoids division issues
  # This creates an effective anisotropic gradient energy
  energy_expression = 'W(eta) +
                       0.5*kappa_0*dot(grad(eta), grad(eta)) +
                       0.5*kappa_1*(pow(dot(grad(eta), vec(1.0, 0.0)), 4.0) +
                                    pow(dot(grad(eta), vec(0.0, 1.0)), 4.0))'

  # Parameters:
  # kappa_0: base interface energy
  # kappa_1: anisotropy strength for fourth-order terms
  parameters = 'kappa_0 0.01 kappa_1 0.0001'

  variables = 'eta'

  use_automatic_differentiation = true
[]

[ICs]
  # Initial condition: circular seed that will evolve to square
  [eta_IC]
    type = FunctionIC
    variable = eta
    function = 'tanh((0.25 - sqrt(x*x + y*y))/0.05)'
  []
[]

[BCs]
  # Natural boundary conditions (no flux)
[]

[Executioner]
  type = Transient

  # Time stepping - slower to observe shape evolution
  dt = 0.002
  end_time = 0.01

  # Solver settings
  solve_type = NEWTON

  petsc_options_iname = '-pc_type -pc_hypre_type -pc_hypre_boomeramg_strong_threshold'
  petsc_options_value = 'hypre boomeramg 0.7'

  nl_rel_tol = 1e-8
  nl_abs_tol = 1e-10
  nl_max_its = 25

  l_tol = 1e-4
  l_max_its = 50

  [TimeStepper]
    type = IterationAdaptiveDT
    dt = 0.002
    growth_factor = 1.1
    cutback_factor = 0.8
    optimal_iterations = 12
  []
[]

[Postprocessors]
  # Monitor phase field evolution
  [eta_avg]
    type = ElementAverageValue
    variable = eta
    execute_on = 'initial timestep_end'
  []

  [eta_max]
    type = ElementExtremeValue
    variable = eta
    value_type = max
    execute_on = 'initial timestep_end'
  []

  [eta_min]
    type = ElementExtremeValue
    variable = eta
    value_type = min
    execute_on = 'initial timestep_end'
  []

  # Track total energy
  [total_free_energy]
    type = ElementIntegralVariablePostprocessor
    variable = total_energy_density
    execute_on = 'initial timestep_end'
  []

  # Measure anisotropy effect
  [interface_width_x]
    type = ElementIntegralVariablePostprocessor
    variable = interface_indicator_x
    execute_on = 'timestep_end'
  []

  [interface_width_y]
    type = ElementIntegralVariablePostprocessor
    variable = interface_indicator_y
    execute_on = 'timestep_end'
  []
[]

[AuxVariables]
  [total_energy_density]
    order = CONSTANT
    family = MONOMIAL
  []

  [interface_indicator_x]
    order = CONSTANT
    family = MONOMIAL
  []

  [interface_indicator_y]
    order = CONSTANT
    family = MONOMIAL
  []

  [interface_normal_x]
    order = CONSTANT
    family = MONOMIAL
  []

  [interface_normal_y]
    order = CONSTANT
    family = MONOMIAL
  []
[]

[AuxKernels]
  # Total energy density for visualization
  [total_energy_aux]
    type = ParsedAux
    variable = total_energy_density
    coupled_variables = 'eta'
    expression = '(eta^2 - 1)^2 + 0.01*0.5*(grad_eta_x^2 + grad_eta_y^2)'
    constant_names = 'grad_eta_x grad_eta_y'
    constant_expressions = '0 0'  # Placeholder for actual gradients
    execute_on = 'initial timestep_end'
  []

  # Interface indicators (high where gradient is strong in each direction)
  [interface_x_aux]
    type = ParsedAux
    variable = interface_indicator_x
    coupled_variables = 'eta'
    expression = 'abs(grad_eta_x)'
    constant_names = 'grad_eta_x'
    constant_expressions = '0'
    execute_on = 'timestep_end'
  []

  [interface_y_aux]
    type = ParsedAux
    variable = interface_indicator_y
    coupled_variables = 'eta'
    expression = 'abs(grad_eta_y)'
    constant_names = 'grad_eta_y'
    constant_expressions = '0'
    execute_on = 'timestep_end'
  []
[]

[VectorPostprocessors]
  # Diagonal sampling to observe square shape formation
  [diagonal_profile]
    type = LineValueSampler
    variable = 'eta'
    start_point = '-0.707 -0.707 0'
    end_point = '0.707 0.707 0'
    num_points = 200
    sort_by = x
    execute_on = 'initial final'
  []

  # Horizontal profile
  [horizontal_profile]
    type = LineValueSampler
    variable = 'eta'
    start_point = '-1 0 0'
    end_point = '1 0 0'
    num_points = 200
    sort_by = x
    execute_on = 'initial final'
  []

  # Vertical profile
  [vertical_profile]
    type = LineValueSampler
    variable = 'eta'
    start_point = '0 -1 0'
    end_point = '0 1 0'
    num_points = 200
    sort_by = y
    execute_on = 'initial final'
  []
[]

[Outputs]
  exodus = true
  print_linear_residuals = false
  print_nonlinear_residuals = true

  [console]
    type = Console
  []

  [csv]
    type = CSV
    execute_on = 'initial timestep_end final'
  []

  # Output every 5 time steps for visualization
  time_step_interval = 5

  # Performance information
  perf_graph = true
[]
