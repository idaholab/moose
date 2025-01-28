##########################################################
# Lid-driven cavity test
# Reynolds: 5,000
# Author: Dr. Mauricio Tano
# Last Update: May, 2024
# Turbulent model using:
# k-epsilon model with capped mixing length
# Standard wall functions
# SIMPLE Solve
##########################################################

### Thermophysical Properties ###
mu = 2e-5
rho = 1.0

### Operation Conditions ###
lid_velocity = 1.0
side_length = 0.1

### k-epsilon Closure Parameters ###
sigma_k = 1.0
sigma_eps = 1.3
C1_eps = 1.44
C2_eps = 1.92
C_mu = 0.09
C_pl = 0.1

### Initial Conditions ###
intensity = 0.01
k_init = '${fparse 1.5*(intensity * lid_velocity)^2}'
eps_init = '${fparse C_mu^0.75 * k_init^1.5 / side_length}'
mu_t_init = '${fparse rho * C_mu * k_init * k_init / eps_init}'

### Modeling parameters ###
walls = 'left top right bottom'
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
    nx = 12
    ny = 12
  []
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

        wall_boundaries = 'top left right bottom'
        momentum_wall_types = 'noslip noslip noslip noslip'
        momentum_wall_functors = '${lid_velocity} 0; 0 0; 0 0; 0 0'

        pin_pressure = true
        pinned_pressure_type = point-value-uo
        pinned_pressure_value = 0
        pinned_pressure_point = '0.01 0.099 0.0'

        momentum_two_term_bc_expansion = false
        pressure_two_term_bc_expansion = false
      []
    []
    [Turbulence]
      [keps]
        turbulence_handling = 'k-epsilon'

        tke_name = TKE
        tked_name = TKED

        # Initialization
        initial_tke = ${k_init}
        initial_tked = ${eps_init}
        initial_mu_t = ${mu_t_init}

        # Model parameters
        C1_eps = ${C1_eps}
        C2_eps = ${C2_eps}
        C_mu = ${C_mu}
        C_pl = ${C_pl}

        sigma_k = ${sigma_k}
        sigma_eps = ${sigma_eps}

        # Wall parameters
        turbulence_walls = ${walls}
        bulk_wall_treatment = ${bulk_wall_treatment}
        wall_treatment_eps = ${wall_treatment}

        # Numerical parameters
        turbulent_viscosity_two_term_bc_expansion = false
        mu_t_as_aux_variable = true
      []
    []
  []
[]

[AuxVariables]
  [dummy]
    type = MooseVariableConstMonomial
  []
[]
[Bounds]
  [min_tke]
    type = ConstantBounds
    variable = dummy
    bound_value = 1e-8
    bounded_variable = TKE
    bound_type = lower
  []
  [min_eps]
    type = ConstantBounds
    variable = dummy
    bound_value = 1e-8
    bounded_variable = TKED
    bound_type = lower
  []
[]

[Executioner]
  type = Steady
  solve_type = 'NEWTON'
  petsc_options_iname = '-pc_type -pc_factor_shift_type -snes_type'
  petsc_options_value = 'lu        NONZERO              vinewtonrsls '
  nl_abs_tol = 1e-7
  nl_rel_tol = 1e-8
  nl_max_its = 100
  line_search = none
  automatic_scaling = true
[]

[Outputs]
  exodus = true
  csv = false
  perf_graph = false
  print_nonlinear_residuals = true
  print_linear_residuals = false
[]
