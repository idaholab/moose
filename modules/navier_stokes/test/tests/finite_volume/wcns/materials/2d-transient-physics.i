l = 10
advected_interp_method = 'average'

# Operating conditions
inlet_temp = 300
outlet_pressure = 1e5
inlet_v = 0.001

[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    xmin = 0
    xmax = ${l}
    ymin = 0
    ymax = 1
    nx = 20
    ny = 10
  []
[]

[Physics]
  [NavierStokes]
    [Flow]
      [flow]
        compressibility = 'weakly-compressible'
        fp = 'fp'

        velocity_variable = 'u v'
        fluid_temperature_variable = 'T'

        density = 'rho'
        dynamic_viscosity = 'mu'
        mu_rampdown = 'mu_rampdown'

        initial_velocity = '${inlet_v} 0 0'
        initial_pressure = '${outlet_pressure}'

        inlet_boundaries = 'left'
        momentum_inlet_types = 'fixed-velocity'
        momentum_inlet_functors = '${inlet_v} 0'

        wall_boundaries = 'top bottom'
        momentum_wall_types = 'noslip noslip'

        outlet_boundaries = 'right'
        momentum_outlet_types = 'fixed-pressure'
        pressure_functors = '${outlet_pressure}'

        mass_advection_interpolation = ${advected_interp_method}
        momentum_advection_interpolation = ${advected_interp_method}
      []
    []
    [FluidHeatTransfer]
      [energy]
        coupled_flow_physics = flow
        fluid_temperature_variable = 'T'

        fp = 'fp'
        thermal_conductivity = 'k'
        specific_heat = 'cp'

        initial_temperature = '${inlet_temp}'

        energy_inlet_types = 'fixed-temperature'
        energy_inlet_functors = '${inlet_temp}'
        energy_wall_types = 'heatflux heatflux'
        energy_wall_functors = '0 0'

        external_heat_source = 'power_density'
        energy_advection_interpolation = 'average'
      []
    []
    [Turbulence]
      [turbulence]
        coupled_flow_physics = flow
        fluid_heat_transfer_physics = energy
      []
    []
  []
[]

[AuxVariables]
  [velocity_norm]
    type = MooseVariableFVReal
  []
  [power_density]
    type = MooseVariableFVReal
    initial_condition = 1e4
  []
[]

[FluidProperties]
  [fp]
    type = FlibeFluidProperties
    # AD-version of h_from_p_T(p, T, h, dh_dp, dh_dT) not implemented
    allow_imperfect_jacobians = true
  []
[]

[AuxKernels]
  [speed]
    type = VectorMagnitudeAux
    variable = 'velocity_norm'
    x = u
    y = v
  []
[]

[Functions]
  [mu_rampdown]
    type = PiecewiseLinear
    x = '1 2 3 4'
    y = '1e3 1e2 1e1 1'
  []
[]

[Executioner]
  type = Transient
  solve_type = 'NEWTON'
  petsc_options_iname = '-pc_type -pc_factor_shift_type'
  petsc_options_value = 'lu       NONZERO'

  [TimeStepper]
    type = IterationAdaptiveDT
    dt = 1e-3
    optimal_iterations = 6
  []
  end_time = 15

  nl_abs_tol = 1e-12
  nl_max_its = 50
  line_search = 'none'

  automatic_scaling = true
  off_diagonals_in_auto_scaling = true
  compute_scaling_once = false
[]

[Outputs]
  exodus = true
[]
