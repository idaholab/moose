coupled_ch_ac_simple.i# Simple Coupled Cahn-Hilliard and Allen-Cahn Test
#
# Minimal example for testing coupled conserved and non-conserved dynamics
#
# Energy functional:
# F[c,eta] = ∫ [W_c(c) + κ_c/2|∇c|² + W_η(eta) + κ_η/2|∇eta|² + g*c*eta] dx
#
# Evolution:
# - c: Conserved (Cahn-Hilliard dynamics)
#   ∂c/∂t = ∇·(M∇μ) where μ = δF/δc
#
# - eta: Non-conserved (Allen-Cahn dynamics)
#   ∂eta/∂t = -L*δF/δeta

[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 30
  ny = 30
  xmin = 0
  xmax = 5
  ymin = 0
  ymax = 5
[]

[Variables]
  [c]
    # Concentration - conserved order parameter
  []
  [eta]
    # Phase field - non-conserved order parameter
  []
[]

[AutomaticWeakForm]
  energy_type = EXPRESSION

  # Simple coupled energy functional
  # Double-well potentials: (c²-1)² and (η²-1)²
  # Gradient energies: κ/2|∇c|² and κ/2|∇η|²
  # Coupling: g*c*η encourages c and η to have same sign
  energy_expression = '(c*c - 1.0)*(c*c - 1.0) +
                       (eta*eta - 1.0)*(eta*eta - 1.0) +
                       0.5*kappa*dot(grad(c), grad(c)) +
                       0.5*kappa*dot(grad(eta), grad(eta)) +
                       g*c*eta'

  # Use same gradient coefficient for simplicity
  parameters = 'kappa 1.0 g 0.5'

  variables = 'c eta'

  use_automatic_differentiation = true
[]

[ICs]
  # Random perturbation for c
  [c_IC]
    type = RandomIC
    variable = c
    min = -0.05
    max = 0.05
    seed = 100
  []

  # Localized perturbation for eta
  [eta_IC]
    type = FunctionIC
    variable = eta
    function = '0.5*exp(-((x-2.5)^2 + (y-2.5)^2))'
  []
[]

[Executioner]
  type = Transient

  # Time stepping
  dt = 0.001
  end_time = 1.0

  # Solver settings
  solve_type = NEWTON

  petsc_options_iname = '-pc_type'
  petsc_options_value = 'lu'

  nl_rel_tol = 1e-8
  nl_abs_tol = 1e-10
  nl_max_its = 15

  l_tol = 1e-5
  l_max_its = 30
[]

[Postprocessors]
  # Check conservation of c
  [total_c]
    type = ElementIntegralVariablePostprocessor
    variable = c
    execute_on = 'initial timestep_end'
  []

  # Monitor eta (should not be conserved)
  [total_eta]
    type = ElementIntegralVariablePostprocessor
    variable = eta
    execute_on = 'initial timestep_end'
  []

  # Monitor energy decrease
  [total_free_energy]
    type = ElementIntegralVariablePostprocessor
    variable = free_energy
    execute_on = 'initial timestep_end'
  []
[]

[AuxVariables]
  [free_energy]
    order = CONSTANT
    family = MONOMIAL
  []
[]

[AuxKernels]
  [free_energy_density]
    type = ParsedAux
    variable = free_energy
    coupled_variables = 'c eta'
    expression = '(c^2 - 1)^2 + (eta^2 - 1)^2 + 0.5*c*eta'
    execute_on = 'initial timestep_end'
  []
[]

[Outputs]
  exodus = true

  [console]
    type = Console
  []

  [csv]
    type = CSV
    execute_on = 'initial timestep_end final'
  []

  time_step_interval = 10
[]