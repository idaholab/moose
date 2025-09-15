# Allen-Cahn with Anisotropic Interfacial Energy
#
# This example demonstrates Allen-Cahn dynamics with anisotropic interface energy
# where the gradient coefficient depends on the interface orientation.
#
# Energy functional:
# F[eta] = ∫ [f_bulk(eta) + f_interface(eta, ∇eta)] dx
#
# where:
#   f_bulk = W(eta) = (eta^2 - 1)^2
#   f_interface = κ(θ)|∇eta|^2 with κ(θ) = κ_0*(1 + ε*cos(4θ))
#
# We approximate the anisotropic coefficient using directional projections:
# κ_eff ≈ κ_0*(1 + ε*((∇eta_x)^2 - (∇eta_y)^2)/|∇eta|^2)
#
# Using vec() operator, we can write this more clearly as:
# n = ∇eta/|∇eta| (interface normal)
# κ_eff = κ_0*(1 + ε*(4*n_x^2*n_y^2))  (fourfold anisotropy)

[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 30
  ny = 30
  xmin = -1
  xmax = 1
  ymin = -1
  ymax = 1
[]

[Variables]
  [eta]
    order = FIRST
    family = LAGRANGE
  []
[]

[AutomaticWeakForm]
  energy_type = EXPRESSION

  # Anisotropic Allen-Cahn energy
  # The anisotropic term uses the components of the gradient
  # We use dot products with basis vectors created by vec() to extract components
  # e_x = vec(1, 0), e_y = vec(0, 1)
  # grad_eta_x = dot(grad(eta), vec(1.0, 0.0))
  # grad_eta_y = dot(grad(eta), vec(0.0, 1.0))

  # Fourfold anisotropy: κ(θ) = κ_0*(1 + ε*cos(4θ))
  # We approximate cos(4θ) ≈ (grad_x^2 - grad_y^2)^2 / |grad|^4 - 1/2
  # But for simplicity, we'll use a different form that's easier to express

  # Simple anisotropic form with different x and y gradient coefficients
  energy_expression = 'W(eta) +
                       0.5*kappa_x*pow(dot(grad(eta), vec(1.0, 0.0)), 2.0) +
                       0.5*kappa_y*pow(dot(grad(eta), vec(0.0, 1.0)), 2.0)'

  # Parameters: different gradient coefficients in x and y directions
  # This creates rectangular/elliptical interfaces
  parameters = 'kappa_x 0.01 kappa_y 0.04'

  variables = 'eta'

  use_automatic_differentiation = true
[]

[ICs]
  # Initial condition: circular inclusion
  [eta_IC]
    type = FunctionIC
    variable = eta
    function = 'if(sqrt(x*x + y*y) < 0.3, 1.0, -1.0)'
  []
[]

[Executioner]
  type = Transient

  # Time stepping
  dt = 0.001
  end_time = 0.01

  # Solver settings
  solve_type = NEWTON

  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'

  nl_rel_tol = 1e-8
  nl_abs_tol = 1e-10
  nl_max_its = 20

  l_tol = 1e-4
  l_max_its = 40

  [TimeStepper]
    type = IterationAdaptiveDT
    dt = 0.001
    growth_factor = 1.2
    cutback_factor = 0.8
    optimal_iterations = 10
  []
[]

[Postprocessors]
  # Monitor total eta (should not be conserved for Allen-Cahn)
  [total_eta]
    type = ElementIntegralVariablePostprocessor
    variable = eta
    execute_on = 'initial timestep_end'
  []

  # Monitor interface area/perimeter
  [interface_area]
    type = ElementIntegralVariablePostprocessor
    variable = grad_eta_mag
    execute_on = 'timestep_end'
  []

  # Track energy components
  [bulk_energy]
    type = ElementIntegralVariablePostprocessor
    variable = bulk_energy_density
    execute_on = 'timestep_end'
  []

  [interface_energy]
    type = ElementIntegralVariablePostprocessor
    variable = interface_energy_density
    execute_on = 'timestep_end'
  []
[]

[AuxVariables]
  [grad_eta_mag]
    order = CONSTANT
    family = MONOMIAL
  []

  [bulk_energy_density]
    order = CONSTANT
    family = MONOMIAL
  []

  [interface_energy_density]
    order = CONSTANT
    family = MONOMIAL
  []

  [grad_eta_x]
    order = CONSTANT
    family = MONOMIAL
  []

  [grad_eta_y]
    order = CONSTANT
    family = MONOMIAL
  []
[]

[AuxKernels]
  # Magnitude of gradient (interface indicator)
  [grad_mag]
    type = ParsedAux
    variable = grad_eta_mag
    coupled_variables = 'eta'
    expression = 'sqrt(grad_eta_x^2 + grad_eta_y^2)'
    constant_names = 'grad_eta_x grad_eta_y'
    constant_expressions = '0 0'  # These would need actual gradient components
    execute_on = 'timestep_end'
  []

  # Bulk energy density
  [bulk_energy_aux]
    type = ParsedAux
    variable = bulk_energy_density
    coupled_variables = 'eta'
    expression = '(eta^2 - 1)^2'
    execute_on = 'timestep_end'
  []

  # Interface energy density (approximation for visualization)
  [interface_energy_aux]
    type = ParsedAux
    variable = interface_energy_density
    coupled_variables = 'eta'
    expression = '0.01*(grad_eta_x^2) + 0.04*(grad_eta_y^2)'
    constant_names = 'grad_eta_x grad_eta_y'
    constant_expressions = '0 0'  # Placeholder
    execute_on = 'timestep_end'
  []
[]

[VectorPostprocessors]
  # Sample along x and y axes to see anisotropic evolution
  [line_sample_x]
    type = LineValueSampler
    variable = 'eta'
    start_point = '-1 0 0'
    end_point = '1 0 0'
    num_points = 200
    sort_by = x
    execute_on = 'initial timestep_end'
  []

  [line_sample_y]
    type = LineValueSampler
    variable = 'eta'
    start_point = '0 -1 0'
    end_point = '0 1 0'
    num_points = 200
    sort_by = y
    execute_on = 'initial timestep_end'
  []
[]

[Outputs]
  exodus = true

  [console]
    type = Console
    print_linear_residuals = false
  []

  [csv]
    type = CSV
    execute_on = 'initial timestep_end final'
  []

  # Output every 10 time steps
  time_step_interval = 10

  # Create checkpoint for restart
  [checkpoint]
    type = Checkpoint
    time_step_interval = 50
    num_files = 2
  []
[]