##########################################################
# Lid-driven cavity test
# Reynolds: 5,000
# Author: Dr. Mauricio Tano & Guillaume Giudicelli
# Last Update: July 2024
# Turbulent model using:
# k-epsilon model
# Standard wall functions with temperature wall functions
# Physics-based syntax
# Fully coupled solve
##########################################################

# Note:
# - in the original input k_t is lagged

### Thermophysical Properties ###
mu = 2e-5
rho = 1.0
k = 0.01
cp = 10.0
Pr_t = 0.9

### Operation Conditions ###
lid_velocity = 1.0
side_length = 0.1

### k-epsilon Closure Parameters ###
sigma_k = 1.0
sigma_eps = 1.3
C1_eps = 1.44
C2_eps = 1.92
C_mu = 0.09

### Initial Conditions ###
intensity = 0.01
k_init = '${fparse 1.5*(intensity * lid_velocity)^2}'
eps_init = '${fparse C_mu^0.75 * k_init^1.5 / side_length}'
mu_t_init = '${fparse rho * C_mu * k_init * k_init / eps_init}'

### Modeling parameters ###
bulk_wall_treatment = false
walls = 'left top right bottom'
wall_treatment_eps = 'eq_newton' # Options: eq_newton, eq_incremental, eq_linearized, neq
wall_treatment_tem = 'eq_linearized' # Options: eq_newton, eq_incremental, eq_linearized, neq

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

        # Material properties
        density = ${rho}
        dynamic_viscosity = ${mu}

        # Initial conditions
        initial_pressure = 0.2
        # Need a non-zero starting velocity to avoid
        # sparsity pattern + Newton error
        initial_velocity = '1e-10 1e-10 0'

        wall_boundaries = 'left right bottom top'
        momentum_wall_types = 'noslip noslip noslip noslip'
        momentum_wall_functors = '0 0; 0 0; 0 0; ${lid_velocity} 0'

        pin_pressure = true
        pinned_pressure_type = point-value
        pinned_pressure_value = 0
        pinned_pressure_point = '0.01 0.099 0.0'

        # Numerical parameters
        momentum_two_term_bc_expansion = false
        pressure_two_term_bc_expansion = false
      []
    []
    [FluidHeatTransfer]
      [energy]
        coupled_flow_physics = flow

        thermal_conductivity = ${k}
        specific_heat = ${cp}

        initial_temperature = '1'

        # Energy turbulent boundary conditions are specified in
        # the turbulence block
        energy_wall_types = 'heatflux heatflux wallfunction wallfunction'
        energy_wall_functors = '0 0 0 1'

        # Numerical parameters
        energy_advection_interpolation = 'average'
        energy_two_term_bc_expansion = false
      []
    []
    [Turbulence]
      [keps]
        fluid_heat_transfer_physics = energy
        turbulence_handling = 'k-epsilon'

        tke_name = TKE
        tked_name = TKED

        # Initialization
        initial_tke = ${k_init}
        initial_tked = ${eps_init}
        initial_mu_t = ${mu_t_init}

        # Fluid properties
        Pr_t = ${Pr_t}

        # Model parameters
        C1_eps = ${C1_eps}
        C2_eps = ${C2_eps}
        C_mu = ${C_mu}
        sigma_k = ${sigma_k}
        sigma_eps = ${sigma_eps}

        # Wall parameters
        turbulence_walls = ${walls}
        bulk_wall_treatment = ${bulk_wall_treatment}
        wall_treatment_eps = ${wall_treatment_eps}
        wall_treatment_T = ${wall_treatment_tem}

        # Numerical parameters
        turbulent_viscosity_two_term_bc_expansion = false
        mu_t_as_aux_variable = true
        k_t_as_aux_variable = true
        # this case requires it for convergence
        linearize_sink_sources = true
        neglect_advection_derivatives = true
      []
    []
  []
[]

[Executioner]
  type = Steady
  petsc_options_iname = '-pc_type -pc_factor_shift_type'
  petsc_options_value = 'lu NONZERO'
  nl_rel_tol = 1e-10
  nl_abs_tol = 5e-8
  nl_max_its = 100

  # Necessary for these cases
  snesmf_reuse_base = false
  line_search = none
[]

[Outputs]
  exodus = true
  csv = false
  perf_graph = false
[]
