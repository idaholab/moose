# Coupled Cahn-Hilliard and Allen-Cahn with Anisotropic Interface Energy
#
# This advanced example couples Cahn-Hilliard and Allen-Cahn dynamics
# with anisotropic interface energy that depends on the interface orientation.
#
# Physical system: Spinodal decomposition with grain growth
# - c: Concentration field (conserved) - represents alloy composition
# - eta: Phase field (non-conserved) - represents grain orientation
#
# Energy functional with anisotropic effects:
# F[c,eta] = ∫ [f_bulk + f_interface + f_coupling] dx
#
# where:
#   f_bulk = W_c(c) + W_eta(eta)
#   f_interface = κ_c(θ)|∇c|^2 + κ_eta|∇eta|^2
#   f_coupling = λ*c^2*(1-eta^2) + α*c*eta
#
# κ_c(θ) = κ_0*(1 + ε*cos(4*θ)) is the anisotropic coefficient
# where θ = atan2(∇c_y, ∇c_x) is the interface orientation

[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 50
  ny = 50
  xmin = 0
  xmax = 20
  ymin = 0
  ymax = 20
[]

[Variables]
  [c]
    order = FIRST
    family = LAGRANGE
    scaling = 1.0
  []
  [eta]
    order = FIRST
    family = LAGRANGE
    scaling = 1.0
  []
[]

[AutomaticWeakForm]
  energy_type = EXPRESSION

  # Energy functional with anisotropic interface energy
  # Note: We approximate the anisotropic term using a Taylor expansion
  # κ(θ) ≈ κ_0*(1 + ε*(2*(∇c_x)^2 - (∇c_y)^2)/|∇c|^2) for small anisotropy
  #
  # Full energy includes:
  # 1. Double-well potentials for both fields
  # 2. Gradient energies (with anisotropy for c)
  # 3. Coupling terms that link composition to phase

  # NOTE: Full anisotropic interface energy would require:
  # κ(θ) = κ_0*(1 + ε*cos(4θ)) where θ = atan2(grad_c_y, grad_c_x)
  # This needs vector component access: grad(c)[0], grad(c)[1]
  # or dot products with basis vectors: dot(grad(c), e_x)
  # These features are planned for future implementation.
  #
  # For now, we use a simpler coupled model with isotropic interfaces
  # but with spatially varying gradient coefficient

  energy_expression = '
    0.25*(c*c - 1.0)*(c*c - 1.0) +
    0.25*(eta*eta - 1.0)*(eta*eta - 1.0) +
    0.5*kappa_c*dot(grad(c), grad(c)) +
    0.5*kappa_eta*dot(grad(eta), grad(eta)) +
    lambda*c*c*(1.0 - eta*eta) +
    alpha*c*eta +
    epsilon*kappa_c*eta*eta*dot(grad(c), grad(c))'

  # Parameters:
  # kappa_c, kappa_eta: Interface energies
  # lambda: Coupling strength (composition-phase)
  # alpha: Linear coupling
  # epsilon: Anisotropy strength (small)
  # delta: Regularization to avoid division by zero
  parameters = 'kappa_c 0.8 kappa_eta 0.5 lambda 1.5 alpha 0.5 epsilon 0.1 delta 1e-6'

  variables = 'c eta'

  use_automatic_differentiation = true

  # Can enable for debugging
  verbose = false
[]

[ICs]
  # Create initial spinodal pattern for concentration
  [c_IC]
    type = FunctionIC
    variable = c
    function = '0.0 + 0.05*cos(0.5*x)*cos(0.7*y) + 0.025*cos(1.2*x)*cos(0.9*y)'
  []

  # Create initial grain structure for eta
  [eta_IC]
    type = FunctionIC
    variable = eta
    function = 'tanh((sin(0.3*x)*cos(0.3*y) - 0.1)/0.1)'
  []
[]

# For Cahn-Hilliard, we need to solve for the chemical potential
# The AutomaticWeakForm handles this automatically through the
# variational derivative, but we can add a split formulation if needed

[BCs]
  # Periodic boundary conditions for both fields
  [Periodic]
    [all]
      variable = 'eta c'
      auto_direction = 'x y'
    []
  []
[]

[Preconditioning]
  [smp]
    type = SMP
    full = true
    petsc_options_iname = '-pc_type -sub_pc_type -pc_asm_overlap'
    petsc_options_value = 'asm      lu           1'
  []
[]

[Executioner]
  type = Transient

  # Adaptive time stepping
  [TimeStepper]
    type = IterationAdaptiveDT
    dt = 0.01
    cutback_factor = 0.5
    growth_factor = 1.2
    optimal_iterations = 7
  []

  end_time = 20.0

  solve_type = NEWTON

  nl_rel_tol = 1e-8
  nl_abs_tol = 1e-10
  nl_max_its = 25

  l_tol = 1e-4
  l_max_its = 50
[]

[Postprocessors]
  # Conservation check for c
  [mass_conservation]
    type = ElementIntegralVariablePostprocessor
    variable = c
    execute_on = 'initial timestep_end'
  []

  # Average values
  [avg_c]
    type = ElementAverageValue
    variable = c
    execute_on = 'initial timestep_end'
  []

  [avg_eta]
    type = ElementAverageValue
    variable = eta
    execute_on = 'initial timestep_end'
  []

  # Energy components
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

  # # Interface width measurement
  # [interface_width]
  #   type = InterfaceWidth
  #   variable = c
  #   execute_on = 'timestep_end'
  # []
[]

[AuxVariables]
  [bulk_energy_density]
    order = CONSTANT
    family = MONOMIAL
  []

  [interface_energy_density]
    order = CONSTANT
    family = MONOMIAL
  []

  [coupling_energy_density]
    order = CONSTANT
    family = MONOMIAL
  []
[]

[AuxKernels]
  # Visualize different energy contributions
  [bulk_energy_aux]
    type = ParsedAux
    variable = bulk_energy_density
    coupled_variables = 'c eta'
    expression = '0.25*(c^2 - 1)^2 + 0.25*(eta^2 - 1)^2'
    execute_on = 'timestep_end'
  []

  [interface_energy_aux]
    type = ParsedAux
    variable = interface_energy_density
    coupled_variables = 'c eta'
    # Note: This is approximate - doesn't include anisotropy visualization
    expression = '0.4*(grad_c_x^2 + grad_c_y^2) + 0.25*(grad_eta_x^2 + grad_eta_y^2)'
    constant_names = 'grad_c_x grad_c_y grad_eta_x grad_eta_y'
    constant_expressions = '0 0 0 0'  # Placeholder - gradients not directly accessible
    execute_on = 'timestep_end'
  []

  [coupling_energy_aux]
    type = ParsedAux
    variable = coupling_energy_density
    coupled_variables = 'c eta'
    expression = '1.5*c^2*(1 - eta^2) + 0.5*c*eta'
    execute_on = 'timestep_end'
  []
[]

[VectorPostprocessors]
  # Sample along diagonal
  [diagonal_profile]
    type = LineValueSampler
    variable = 'c eta'
    start_point = '0 0 0'
    end_point = '20 20 0'
    num_points = 200
    sort_by = x
    execute_on = 'initial final'
  []
[]

[Outputs]
  exodus = true

  [console]
    type = Console
    print_linear_residuals = false
    print_nonlinear_residuals = false
  []

  [checkpoint]
    type = Checkpoint
    time_step_interval = 50
    num_files = 2
  []

  # Output every 5 time steps
  time_step_interval = 5

  # Performance information
  perf_graph = true
[]
