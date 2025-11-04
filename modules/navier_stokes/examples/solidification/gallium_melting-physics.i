##########################################################
# Simulation of Gallium Melting Experiment
# Ref: Gau, C., & Viskanta, R. (1986). Melting and solidification of a pure metal on a vertical wall.
# Key physics: melting/solidification, convective heat transfer, natural convection
##########################################################

mu = 1.81e-3
rho_solid = 6093
rho_liquid = 6093
k_solid = 32
k_liquid = 32
cp_solid = 381.5
cp_liquid = 381.5
L = 80160
alpha_b = 1.2e-4
T_solidus = 302.93
T_liquidus = '${fparse T_solidus + 0.1}'
advected_interp_method = 'upwind'
velocity_interp_method = 'rc'
T_cold = 301.15
T_hot = 311.15
Nx = 100
Ny = 50

[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    xmin = 0
    xmax = 88.9e-3
    ymin = 0
    ymax = 63.5e-3
    nx = ${Nx}
    ny = ${Ny}
  []
[]

[AuxVariables]
  [liquid_fraction]
    type = MooseVariableFVReal
    initial_condition = 0.0
  []
[]

[AuxKernels]
  # Note that because this is only executed on timestep end,
  # the phase fractions are constant over each time step
  [compute_fl]
    type = NSLiquidFractionAux
    variable = 'liquid_fraction'
    temperature = T_fluid
    T_liquidus = '${T_liquidus}'
    T_solidus = '${T_solidus}'
    execute_on = 'TIMESTEP_END'
  []
[]

[Physics]
  [NavierStokes]
    [Flow]
      [flow]
        compressibility = 'incompressible'

        pin_pressure = true
        pinned_pressure_type = 'average'
        pinned_pressure_value = 0

        velocity_variable = 'vel_x vel_y'

        density = 'rho_mixture'
        dynamic_viscosity = ${mu}

        # Initial conditions
        initial_velocity = '0 0 0'
        initial_pressure = 0

        # Boundary conditions
        wall_boundaries = 'left right top bottom'
        momentum_wall_types = 'noslip noslip noslip noslip'

        # Friction
        friction_types = "Darcy Forchheimer"
        friction_coeffs = "Darcy_coefficient Forchheimer_coefficient"

        # Boussinesq
        boussinesq_approximation = true
        gravity = '0 -9.81 0'
        ref_temperature = ${T_cold}
        thermal_expansion = 'alpha_b'
        # Solid phase is not moving
        density_for_gravity_terms = ${rho_liquid}

        mass_advection_interpolation = '${advected_interp_method}'
        momentum_advection_interpolation = '${advected_interp_method}'
        velocity_interpolation = '${velocity_interp_method}'
      []
    []
    [FluidHeatTransfer]
      [energy]
        coupled_flow_physics = flow

        thermal_conductivity = 'k_mixture'
        specific_heat = 'cp_mixture'

        initial_temperature = '${T_cold}'

        # See flow physics for wall names
        energy_wall_types = 'fixed-temperature fixed-temperature heatflux heatflux'
        energy_wall_functors = '${T_hot} ${T_cold} 0 0'

        energy_advection_interpolation = '${advected_interp_method}'
      []
    []
    [TwoPhaseMixture]
      [mixture]
        add_phase_transport_equation = false
        phase_1_fraction_name = 'liquid_fraction'
        phase_2_fraction_name = 'solid_fraction'

        fluid_heat_transfer_physics = energy
        add_phase_change_energy_term = true
        use_dispersed_phase_drag_model = false

        # Base phase material properties
        phase_1_density_name = ${rho_liquid}
        phase_1_viscosity_name = ${mu}
        phase_1_specific_heat_name = ${cp_liquid}
        phase_1_thermal_conductivity_name = ${k_liquid}
        output_all_properties = true

        # Other phase material properties
        phase_2_density_name = ${rho_solid}
        phase_2_viscosity_name = ${mu}
        phase_2_specific_heat_name = ${cp_solid}
        phase_2_thermal_conductivity_name = ${k_solid}
      []
    []
  []
[]

[FunctorMaterials]
  [mushy_zone_resistance]
    type = INSFVMushyPorousFrictionFunctorMaterial
    liquid_fraction = 'liquid_fraction'
    mu = '${mu}'
    rho_l = '${rho_liquid}'
    dendrite_spacing_scaling = 1e-1
    # We have to use this to make the coefficients vectors
    Darcy_coef_name = Darcy
    Forchheimer_coef_name = Forchheimer
  []
  [friction_coefs]
    type = ADGenericVectorFunctorMaterial
    prop_names = 'Darcy_coefficient Forchheimer_coefficient'
    prop_values = 'Darcy Darcy Darcy Forchheimer Forchheimer Forchheimer'
  []
  [boussinesq_coefficient]
    type = ADGenericFunctorMaterial
    prop_names = 'alpha_b'
    prop_values = '${alpha_b}'
  []
  [latent_heat]
    type = ADGenericFunctorMaterial
    prop_names = 'latent_heat'
    prop_values = '${L}'
  []
  [phase_change_temperature]
    type = ADGenericFunctorMaterial
    prop_names = 'T_solidus T_liquidus'
    prop_values = '${T_solidus} ${T_liquidus}'
  []
[]

[Executioner]
  type = Transient

  # Time-stepping parameters
  start_time = 0.0
  end_time = 200.0
  # num_steps = 2

  [TimeStepper]
    type = IterationAdaptiveDT
    optimal_iterations = 10
    dt = 0.1
  []

  solve_type = 'NEWTON'
  petsc_options_iname = '-pc_type -pc_factor_shift_type'
  petsc_options_value = 'lu NONZERO'
  nl_rel_tol = 1e-2
  nl_abs_tol = 1e-4
  nl_max_its = 30
[]

[Postprocessors]
  [ave_p]
    type = ElementAverageValue
    variable = 'pressure'
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [ave_fl]
    type = ElementAverageValue
    variable = 'liquid_fraction'
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [ave_T]
    type = ElementAverageValue
    variable = 'T_fluid'
    execute_on = 'INITIAL TIMESTEP_END'
  []
[]

[VectorPostprocessors]
  [vel_x_pp]
    type = ElementValueSampler
    variable = 'vel_x liquid_fraction'
    sort_by = 'x'
  []
[]

[Outputs]
  exodus = true
  csv = true
[]
