### Thermophysical Properties ###
rho = 1
cp = 1
Pr_t = 0.9

# large diffusion coefficients to converge without initial conditions (test uses Problem/solve=false)
mu = 100
k = 10
diff_scalar = 10

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
  active = 'gen'
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
  [fmg_restart]
    type = FileMeshGenerator
    file = user_ics.e
    use_for_exodus_restart = true
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

        passive_scalar_diffusivity = '${diff_scalar} ${diff_scalar}'
        passive_scalar_source = '0.1 0'
        passive_scalar_coupled_source = '1; 0'
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

        # Fluid properties
        Pr_t = ${Pr_t}
        Sc_t = '0.7'

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

[Executioner]
  type = Steady
  solve_type = 'NEWTON'
  petsc_options_iname = '-pc_type -pc_factor_shift_type'
  petsc_options_value = 'lu NONZERO'
  nl_abs_tol = 1e-3

  line_search = 'none'
[]

[Problem]
  solve = false
[]

[Outputs]
  # Used to set up a restart from checkpoint
  checkpoint = true
  # Used to set up a restart from exodus file
  [exodus]
    type = Exodus
    execute_on = TIMESTEP_END
  []
  # Used to check results
  csv = true
  execute_on = INITIAL
[]

[Postprocessors]
  [min_vel_x]
    type = ElementExtremeValue
    variable = 'vel_x'
    value_type = 'min'
    execute_on = 'INITIAL'
  []
  [max_vel_x]
    type = ElementExtremeValue
    variable = 'vel_x'
    value_type = 'max'
    execute_on = 'INITIAL'
  []
  [min_vel_y]
    type = ElementExtremeValue
    variable = 'vel_y'
    value_type = 'min'
    execute_on = 'INITIAL'
  []
  [max_vel_y]
    type = ElementExtremeValue
    variable = 'vel_y'
    value_type = 'max'
    execute_on = 'INITIAL'
  []
  [min_pressure]
    type = ElementExtremeValue
    variable = 'pressure'
    value_type = 'min'
    execute_on = 'INITIAL'
  []
  [max_pressure]
    type = ElementExtremeValue
    variable = 'pressure'
    value_type = 'max'
    execute_on = 'INITIAL'
  []
  [min_T_fluid]
    type = ElementExtremeValue
    variable = 'T_fluid'
    value_type = 'min'
    execute_on = 'INITIAL'
  []
  [max_T_fluid]
    type = ElementExtremeValue
    variable = 'T_fluid'
    value_type = 'max'
    execute_on = 'INITIAL'
  []
  [min_scalar1]
    type = ElementExtremeValue
    variable = 'scalar1'
    value_type = 'min'
    execute_on = 'INITIAL'
  []
  [max_scalar1]
    type = ElementExtremeValue
    variable = 'scalar1'
    value_type = 'max'
    execute_on = 'INITIAL'
  []
  [min_scalar2]
    type = ElementExtremeValue
    variable = 'scalar2'
    value_type = 'min'
    execute_on = 'INITIAL'
  []
  [max_scalar2]
    type = ElementExtremeValue
    variable = 'scalar2'
    value_type = 'max'
    execute_on = 'INITIAL'
  []
  [min_mu_t]
    type = ElementExtremeFunctorValue
    functor = 'mu_t'
    value_type = 'min'
    execute_on = 'INITIAL'
  []
  [max_mu_t]
    type = ElementExtremeFunctorValue
    functor = 'mu_t'
    value_type = 'max'
    execute_on = 'INITIAL'
  []
  [min_tke]
    type = ElementExtremeValue
    variable = 'TKE'
    value_type = 'min'
    execute_on = 'INITIAL'
  []
  [max_tke]
    type = ElementExtremeValue
    variable = 'TKE'
    value_type = 'max'
    execute_on = 'INITIAL'
  []
  [min_tked]
    type = ElementExtremeValue
    variable = 'TKED'
    value_type = 'min'
    execute_on = 'INITIAL'
  []
  [max_tked]
    type = ElementExtremeValue
    variable = 'TKED'
    value_type = 'max'
    execute_on = 'INITIAL'
  []
[]
