### Thermophysical Properties ###
mu = 1
rho = 1
k = 1e-3
diff = 1e-3
cp = 1
Pr_t = 0.9

### Simulation parameters
inlet_velocity = 1
side_length = 1

### k-epsilon Closure Parameters ###
sigma_k = 1.0
sigma_eps = 1.3
C1_eps = 1.44
C2_eps = 1.92
C_mu = 0.09

### Initial Conditions ###
intensity = 0.01
k_init = '${fparse 1.5*(intensity * inlet_velocity)^2}'
eps_init = '${fparse C_mu^0.75 * k_init^1.5 / side_length}'
mu_t_init = '${fparse rho * C_mu * k_init * k_init / eps_init}'

### Modeling parameters ###
bulk_wall_treatment = false
wall_treatment_eps = 'eq_newton' # Options: eq_newton, eq_incremental, eq_linearized, neq
wall_treatment_tem = 'eq_newton' # Options: eq_newton, eq_incremental, eq_linearized, neq

[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    xmin = 0
    xmax = 10
    ymin = 0
    ymax = ${side_length}
    nx = 100
    ny = 20
  []
[]

[Physics]
  [NavierStokes]
    [Flow]
      [all_flow]
        compressibility = 'incompressible'

        density = ${rho}
        dynamic_viscosity = ${mu}

        inlet_boundaries = 'left'
        momentum_inlet_types = 'fixed-velocity'
        momentum_inlet_functors = '${inlet_velocity} 0'

        wall_boundaries = 'top bottom'
        momentum_wall_types = 'noslip noslip'

        outlet_boundaries = 'right'
        momentum_outlet_types = 'fixed-pressure'
        pressure_functors = '0'

        mass_advection_interpolation = 'average'
        momentum_advection_interpolation = 'average'
      []
    []
    [FluidHeatTransfer]
      [all_energy]
        thermal_conductivity = ${k}
        specific_heat = ${cp}

        energy_inlet_types = 'fixed-temperature'
        energy_inlet_functors = '1'

        energy_wall_types = 'heatflux heatflux'
        energy_wall_functors = '0 0'

        energy_advection_interpolation = 'average'
      []
    []
    [ScalarTransport]
      [all_scalar]
        passive_scalar_names = 'scalar1 scalar2'

        passive_scalar_diffusivity = '${diff} ${diff}'
        passive_scalar_source = '0.1 0'
        passive_scalar_coupled_source = 'U; 0'
        passive_scalar_coupled_source_coeff = '0.1; 0'

        passive_scalar_inlet_types = 'fixed-value fixed-value'
        passive_scalar_inlet_functors = '1; 0.1'

        passive_scalar_advection_interpolation = 'average'
      []
    []
    [Turbulence]
      [keps]
        fluid_heat_transfer_physics = 'all_energy'
        scalar_transport_physics = 'all_scalar'
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
        turbulence_walls = 'top bottom'
        bulk_wall_treatment = ${bulk_wall_treatment}
        wall_treatment_eps = ${wall_treatment_eps}
        wall_treatment_T = ${wall_treatment_tem}

        # Numerical parameters
        mu_t_as_aux_variable = false
        k_t_as_aux_variable = false
      []
    []
  []
[]

[AuxVariables]
  [U]
    order = CONSTANT
    family = MONOMIAL
    fv = true
  []
[]

[AuxKernels]
  [mag]
    type = VectorMagnitudeAux
    variable = U
    x = vel_x
    y = vel_y
  []
[]

[Executioner]
  type = Steady
  solve_type = 'NEWTON'
  petsc_options_iname = '-pc_type -pc_factor_shift_type'
  petsc_options_value = 'lu NONZERO'
  nl_rel_tol = 1e-12
[]

[Outputs]
  exodus = true
  csv = true
[]
