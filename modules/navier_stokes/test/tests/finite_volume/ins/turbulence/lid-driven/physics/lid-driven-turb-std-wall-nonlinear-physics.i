##########################################################
# Lid-driven cavity test
# Reynolds: 5,000
# Author: Dr. Mauricio Tano
# Last Update: May, 2024
# Turbulent model using:
# k-epsilon model
# No wall functions
# Newton Solve
##########################################################

### Thermophysical Properties ###
mu = 2e-5
rho = 1.0

### Operation Conditions ###
lid_velocity = 1.0
side_length = 0.1

### Initial Conditions ###
intensity = 0.01
k_init = '${fparse 1.5*(intensity * lid_velocity)^2}'
eps_init = '${fparse C_mu^0.75 * k_init^1.5 / side_length}'

### k-epsilon Closure Parameters ###
sigma_k = 1.0
sigma_eps = 1.3
C1_eps = 1.44
C2_eps = 1.92
C_mu = 0.09

### Modeling parameters ###
walls = '' # no walls for turbulence, to get 'standard' walls
bulk_wall_treatment = false
wall_treatment = 'eq_newton' # Options: eq_newton, eq_incremental, eq_linearized, neq

[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    xmin = 0
    xmax = ${side_length}
    ymin = 0
    ymax = ${side_length}
    nx = 10
    ny = 10
  []
[]

[Problem]
  previous_nl_solution_required = true
[]

[Physics]
  [NavierStokes]
    [Flow]
      [flow]
        compressibility = 'incompressible'

        density = ${rho}
        dynamic_viscosity = ${mu}

        initial_pressure = 0.2
        initial_velocity = '1e-10 1e-10 0'

        wall_boundaries = 'left right top bottom'
        momentum_wall_types = 'noslip noslip noslip noslip'
        momentum_wall_functors = '0 0; 0 0; ${lid_velocity} 0; 0 0'

        pin_pressure = true
        pinned_pressure_type = average
        pinned_pressure_value = 0

        mu_interp_method = 'average'
      []
    []
    [Turbulence]
      [keps]
        turbulence_handling = 'k-epsilon'

        # only needed for comparing input syntax with DumpObjectsProblem
        transient = true

        tke_name = TKE
        tked_name = TKED

        # Initialization
        initial_tke = ${k_init}
        initial_tked = ${eps_init}

        # Model parameters
        C1_eps = ${C1_eps}
        C2_eps = ${C2_eps}
        C_mu = ${C_mu}

        sigma_k = ${sigma_k}
        sigma_eps = ${sigma_eps}

        # Wall parameters
        turbulence_walls = ${walls}
        bulk_wall_treatment = ${bulk_wall_treatment}
        wall_treatment_eps = ${wall_treatment}

        # Numerical parameters
        turbulent_viscosity_two_term_bc_expansion = false
        turbulent_viscosity_interp_method = 'average'
        mu_t_as_aux_variable = false
        output_mu_t = false
      []
    []
  []
[]

[FVBCs]
  [walls_TKE]
    type = FVDirichletBC
    boundary = 'left right top bottom'
    variable = TKE
    value = ${k_init}
  []
  [walls_TKED]
    type = FVDirichletBC
    boundary = 'left right top bottom'
    variable = TKED
    value = ${eps_init}
  []
[]

[Executioner]
  type = Transient
  end_time = 200
  dt = 0.01
  # To force it to end on the same step as the gold file
  num_steps = 160
  solve_type = 'NEWTON'
  petsc_options_iname = '-pc_type -pc_factor_shift_type -snes_linesearch_damping'
  petsc_options_value = 'lu        NONZERO               0.5'
  nl_abs_tol = 1e-8
  nl_rel_tol = 1e-8
  nl_max_its = 50
  line_search = none
[]

[Outputs]
  exodus = true
  csv = false
  perf_graph = false
  print_nonlinear_residuals = true
  print_linear_residuals = true
[]
