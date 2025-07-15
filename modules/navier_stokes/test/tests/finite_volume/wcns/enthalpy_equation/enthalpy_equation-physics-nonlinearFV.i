H = 0.015
L = 1
bulk_u = 0.01
p_ref = 101325.0

advected_interp_method = 'upwind'

[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    xmin = 0
    xmax = ${L}
    ymin = -${H}
    ymax = ${H}
    nx = 30
    ny = 15
  []
[]

[Physics]
  [NavierStokes]
    [Flow]
      [flow]
        compressibility = 'weakly-compressible'

        velocity_variable = 'vel_x vel_y'

        density = 'rho'
        dynamic_viscosity = 'mu'

        initial_velocity = '${bulk_u} 0 0'
        initial_pressure = '${p_ref}'

        inlet_boundaries = 'left'
        # momentum_inlet_types = 'fixed-velocity'
        # momentum_inlet_functors = '${bulk_u} 0'
        # momentum_inlet_types = 'flux-velocity'
        # flux_inlet_pps = '${bulk_u}'

        wall_boundaries = 'top bottom'
        momentum_wall_types = 'noslip noslip'

        outlet_boundaries = 'right'
        momentum_outlet_types = 'fixed-pressure'
        pressure_functors = '${p_ref}'

        momentum_advection_interpolation = ${advected_interp_method}
        pressure_two_term_bc_expansion = false
        momentum_two_term_bc_expansion = false
      []
    []
    [FluidHeatTransfer]
      [energy]
        coupled_flow_physics = flow

        solve_for_enthalpy = true
        # There are no fluid temperature (auxiliary) variable when solving for enthalpy
        # with nonlinear finite volume, because we need T_from_p_h to be computed on-the-fly
        # rather than on an auxiliary kernel update which does not preserve automatic differentiation
        # fluid_temperature_variable = 'T_fluid'

        fp = 'lead'
        thermal_conductivity = 'k'
        specific_heat = 'cp'

        initial_temperature = '777'

        energy_inlet_types = 'flux-velocity'
        # specifies inlet temperature, not inlet enthalpy
        energy_inlet_functors = '860'
        energy_wall_boundaries = 'top bottom'
        energy_wall_types = 'fixed-temperature fixed-temperature'
        energy_wall_functors = '950 950'

        energy_advection_interpolation = ${advected_interp_method}
        energy_two_term_bc_expansion = false
      []
    []
  []
[]

[FluidProperties]
  [lead]
    type = LeadFluidProperties
  []
[]

[FunctorMaterials]
  [fluid_props_to_mat_props]
    type = GeneralFunctorFluidProps
    fp = lead
    pressure = ${p_ref}
    T_fluid = 'T_fluid'
    speed = 1
    porosity = 1
    characteristic_length = 1
  []
[]

[Executioner]
  type = Steady
  solve_type = NEWTON

  petsc_options_iname = '-pc_type -pc_factor_shift_type'
  petsc_options_value = 'lu NONZERO'
[]

[Outputs]
  csv = true
[]

[Postprocessors]
  [inlet_T]
    type = SideAverageValue
    variable = 'T_fluid_out'
    boundary = 'left'
  []
  [average_T]
    type = ElementAverageValue
    variable = 'T_fluid_out'
  []
  [outlet_T]
    type = SideAverageValue
    variable = 'T_fluid_out'
    boundary = 'right'
  []
  [pdrop]
    type = PressureDrop
    boundary = 'right left'
    upstream_boundary = 'right'
    downstream_boundary = 'left'
    pressure = 'pressure'
  []
[]
