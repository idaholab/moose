# Coupled Cahn-Hilliard and Allen-Cahn Model
#
# This example couples a conserved order parameter (c) evolving via
# Cahn-Hilliard dynamics with a non-conserved order parameter (eta)
# evolving via Allen-Cahn dynamics.
#
# Physical system: Phase separation (c) coupled with phase transformation (eta)
# - c: Concentration (conserved)
# - eta: Structural order parameter (non-conserved)
#
# Energy functional:
# F[c,eta] = ∫ [f_ch(c,∇c) + f_ac(eta,∇eta) + f_coupling(c,eta)] dx
#
# where:
#   f_ch = (c^2-1)^2 + κ_c/2|∇c|^2        (Cahn-Hilliard part)
#   f_ac = (eta^2-1)^2 + κ_η/2|∇eta|^2    (Allen-Cahn part)
#   f_coupling = λ*c*eta*(1-eta^2)        (Coupling term)
#
# The coupling term favors c=1 in eta=1 phase and c=-1 in eta=-1 phase

[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 40
  ny = 40
  xmin = 0
  xmax = 10
  ymin = 0
  ymax = 10
[]

[Variables]
  [c]
    order = FIRST
    family = LAGRANGE
    # Initial condition will create phase separation
  []
  [eta]
    order = FIRST
    family = LAGRANGE
    # Initial condition will create domain structure
  []
[]

[AutomaticWeakForm]
  energy_type = EXPRESSION

  # Complete coupled energy functional
  # Cahn-Hilliard part: double-well for c + gradient energy
  # Allen-Cahn part: double-well for eta + gradient energy
  # Coupling: Promotes alignment between concentration and phase
  energy_expression = '((c*c - 1.0)*(c*c - 1.0)) +
                       0.5*kappa_c*dot(grad(c), grad(c)) +
                       ((eta*eta - 1.0)*(eta*eta - 1.0)) +
                       0.5*kappa_eta*dot(grad(eta), grad(eta)) +
                       lambda*c*eta*(1.0 - eta*eta)'

  # Material parameters
  parameters = 'kappa_c 0.5 kappa_eta 0.5 lambda 2.0'

  # Both field variables
  variables = 'c eta'

  # Use automatic differentiation for exact Jacobians
  use_automatic_differentiation = true

  # Enable verbose output to see generated kernels
  verbose = false
[]

[ICs]
  # Random initial condition for concentration (phase separation)
  [c_IC]
    type = RandomIC
    variable = c
    min = -0.1
    max = 0.1
    seed = 12345
  []

  # Create initial domains for eta (two regions)
  [eta_IC]
    type = FunctionIC
    variable = eta
    function = 'if(x<5, 0.9, -0.9) + 0.1*sin(2*3.14159*y/10)'
  []
[]

[BCs]
  # Natural boundary conditions (no flux) are automatically satisfied
  # by the variational formulation
[]

[Executioner]
  type = Transient

  # Time stepping
  dt = 0.01
  end_time = 5.0

  # Nonlinear solver settings
  solve_type = NEWTON

  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'

  nl_rel_tol = 1e-8
  nl_abs_tol = 1e-10
  nl_max_its = 20

  l_tol = 1e-4
  l_max_its = 30
[]

[Postprocessors]
  # Monitor total concentration (should be conserved)
  [total_c]
    type = ElementIntegralVariablePostprocessor
    variable = c
    execute_on = 'initial timestep_end'
  []

  # Monitor average eta
  [avg_eta]
    type = ElementAverageValue
    variable = eta
    execute_on = 'initial timestep_end'
  []

  # Monitor total free energy
  [total_energy]
    type = ElementIntegralVariablePostprocessor
    variable = energy_density
    execute_on = 'initial timestep_end'
  []
[]

[AuxVariables]
  [energy_density]
    order = CONSTANT
    family = MONOMIAL
  []
[]

[AuxKernels]
  # Compute and visualize the energy density
  [energy_density_aux]
    type = ParsedAux
    variable = energy_density
    coupled_variables = 'c eta'
    expression = '(c*c - 1)^2 + (eta*eta - 1)^2 + 2.0*c*eta*(1 - eta*eta)'
    execute_on = 'initial timestep_end'
  []
[]

[VectorPostprocessors]
  # Output concentration profile along centerline
  [centerline_c]
    type = LineValueSampler
    variable = 'c eta'
    start_point = '0 5 0'
    end_point = '10 5 0'
    num_points = 100
    sort_by = x
    execute_on = 'initial final'
  []
[]

[Outputs]
  exodus = true

  [console]
    type = Console
    print_linear_residuals = false
    print_nonlinear_residuals = true
  []

  [csv]
    type = CSV
    file_base = coupled_ch_allen_cahn
    execute_on = 'initial timestep_end final'
  []

  # Output at regular intervals
  print_linear_residuals = false
  interval = 10
[]