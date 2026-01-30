# Inputs:
# - mfr_in
# - mfr_out
# - T_in
# - T_out
# - T_air
# - reactor_power
#
# Outputs:
# - air_heatrate; positive value is heat loss from shield
#
# Water inlet/outlet heat rates are calculated as follows:
#   Q = mfr * cp * (T - T_ref)
# where T_ref is taken to be 0. Better would be:
#   Q = mfr * h(T)

mfr_in_initial = 0.1
mfr_out_initial = 0.1
T_in_initial = 270
T_out_initial = 310
T_air_initial = 300
reactor_power_initial = 50e3

cp_water = 4184

T_ambient = 300
initial_T = 300
htc_outer = 5.0

[Mesh]
  [file_mesh]
    type = FileMeshGenerator
    file = 'generate_mesh_in.e'
  []
[]

[SolidProperties]
  [concrete_sp]
    type = ThermalFunctionSolidProperties
    rho = 3600
    cp = 1050
    k = 0.75
  []
  [water_sp]
    type = ThermalFunctionSolidProperties
    rho = 988
    cp = ${cp_water}
    k = 0.6
  []
[]

[Materials]
  [concrete_mat]
    type = ADConstantDensityThermalSolidPropertiesMaterial
    block = concrete
    sp = concrete_sp
    T_ref = ${initial_T}
    temperature = T
  []
  [water_mat]
    type = ADConstantDensityThermalSolidPropertiesMaterial
    block = water
    sp = water_sp
    T_ref = ${initial_T}
    temperature = T
  []
[]

[Variables]
  [T]
    initial_condition = ${initial_T}
  []
[]

[Kernels]
  [time_derivative]
    type = ADHeatConductionTimeDerivative
    variable = T
  []
  [heat_conduction]
    type = ADHeatConduction
    variable = T
    thermal_conductivity = thermal_conductivity
  []
  [water_source]
    type = BodyForce
    variable = T
    block = water
    function = water_source_fn
  []
[]

[BCs]
  [convection_bc]
    type = FunctorNeumannBC
    variable = T
    boundary = walls_outer
    functor = air_heatflux
    flux_is_inward = false
  []
  [reactor_heatflux_bc]
    type = FunctionNeumannBC
    variable = T
    boundary = inner_wall
    function = reactor_heatflux_fn
  []
[]

[Functions]
  [water_source_fn]
    type = ParsedFunction
    symbol_names = 'power volume'
    symbol_values = 'water_heatrate water_volume'
    expression = 'power / volume'
  []
  [reactor_heatflux_fn]
    type = ParsedFunction
    symbol_names = 'power area'
    symbol_values = 'reactor_power inner_wall_area'
    expression = 'power / area'
  []
[]

[FunctorMaterials]
  [air_heatflux_fmat]
    type = ADConvectionHeatFluxFunctorMaterial
    heat_flux_name = air_heatflux
    htc = ${htc_outer}
    T_solid = T
    T_fluid = ${T_ambient}
  []
[]

[Postprocessors]
  [water_volume]
    type = VolumePostprocessor
    block = water
    execute_on = 'INITIAL'
  []
  [inner_wall_area]
    type = AreaPostprocessor
    boundary = inner_wall
    execute_on = 'INITIAL'
  []

  [T_concrete_max]
    type = NodalExtremeValue
    variable = T
    block = concrete
    value_type = max
    execute_on = 'INITIAL TIMESTEP_END'
  []

  [air_heatrate]
    type = ADSideIntegralFunctorPostprocessor
    functor = air_heatflux
    boundary = walls_outer
    functor_argument = qp
    prefactor = -1
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [water_heatrate]
    type = ParsedPostprocessor
    pp_names = 'mfr_in mfr_out T_in T_out'
    pp_symbols = 'mfr_in mfr_out T_in T_out'
    expression = 'mfr_in * ${cp_water} * T_in - mfr_out * ${cp_water} * T_out'
    execute_on = 'INITIAL TIMESTEP_END'
  []

  # These should be transferred into the FMU:
  [reactor_power]
    type = ConstantPostprocessor
    value = ${reactor_power_initial}
  []
  [mfr_in]
    type = ConstantPostprocessor
    value = ${mfr_in_initial}
  []
  [mfr_out]
    type = ConstantPostprocessor
    value = ${mfr_out_initial}
  []
  [T_in]
    type = ConstantPostprocessor
    value = ${T_in_initial}
  []
  [T_out]
    type = ConstantPostprocessor
    value = ${T_out_initial}
  []
  [T_air]
    type = ConstantPostprocessor
    value = ${T_air_initial}
  []
[]

[Preconditioning]
  [pc]
    type = SMP
    full = true
  []
[]

[Controls]
  [web_server]
    type = WebServerControl
    execute_on = 'initial multiapp_fixed_point_end'
  []
[]

[Executioner]
  type = Transient
  dt = 1000
  end_time = ${units 1000 h -> s}

  scheme = implicit-euler
  solve_type = NEWTON

  nl_abs_tol = 1e-8
  nl_rel_tol = 1e-8
  nl_max_its = 10
  l_tol = 1e-5
  l_max_its = 10
[]

[Outputs]
  file_base = 'dome_rss'
  exodus = true
  csv = true
  [console]
    type = Console
    show = 'T_concrete_max reactor_power air_heatrate water_heatrate'
    max_rows = 1
    execute_on = 'INITIAL TIMESTEP_BEGIN FAILED'
  []
[]
