# A layer of silicone oil over a layer of water in a closed tub, heated from the floor and cooled
# from the lid by dT. The side walls are adiabatic.
#
# With dT = 0 the exact solution is rest forever: the reduced pressure p_rgh = p - rho g.x is
# uniform in each layer and changes by (rho_w - rho_o) g y_i across the diffuse interface, through
# which the initial condition integrates it. That matches the discrete equilibrium to second order
# in the cell size, so the case starts with a small kick that decays to the balanced-force error.
#
# Below the onset of convection in either layer the fluid stays at rest and the steady temperature
# is linear in each layer, carrying the same conductive flux through both. Above onset, both layers
# convect: this is the two-layer Rayleigh-Benard tub.

# Nominal properties of water at 20 C and of a 5 cSt polydimethylsiloxane (silicone) oil
rho_w = 998.2
mu_w = 1.002e-3
k_w = 0.598
cp_w = 4182
beta_w = 2.07e-4
rho_o = 913
mu_o = ${fparse 5e-6 * rho_o}
k_o = 0.117
cp_o = 1630
beta_o = 1.05e-3

g = 9.81
T0 = 295
dT = 0
# Empty to leave out surface tension, otherwise the oil-water interfacial tension [N/m]
surface_tension = ''

# An aspect ratio of 4 leaves room for several convection cells
W = 0.08
H = 0.02
y_i = 0.01
nx = 80
ny = 20
# Half width of the tanh interface profile, one cell
w = ${fparse H / ny}

# Series conduction through the two layers
q = ${fparse dT / (y_i / k_w + (H - y_i) / k_o)}
T_interface = ${fparse T0 + 0.5 * dT - q * y_i / k_w}

[Mesh]
  [mesh]
    type = CartesianMeshGenerator
    dim = 2
    dx = '${W}'
    dy = '${H}'
    ix = '${nx}'
    iy = '${ny}'
  []
[]

[Problem]
  linear_sys_names = 'u_system v_system pressure_system alpha_system energy_system'
  previous_nl_solution_required = true
[]

[Physics]
  [NavierStokes]
    [ConservativeDiffuseInterfaceFlowSegregated]
      [flow]
        velocity_variable = 'vel_x vel_y'
        pressure_variable = 'pressure'

        compressibility = 'incompressible'
        density = 'rho_mixture'
        dynamic_viscosity = 'mu_mixture'
        gravity = '0 -${g} 0'
        buoyancy_density = 'rho_buoyancy'
        volume_fraction_functor = 'alpha'
        surface_tension_coefficient = '${surface_tension}'

        initial_velocity = '0 0 0'
        initial_pressure = 'prgh_hydrostatic'

        wall_boundaries = 'bottom top left right'
        momentum_wall_types = 'noslip noslip noslip noslip'

        orthogonality_correction = false
        momentum_two_term_bc_expansion = false
        pressure_two_term_bc_expansion = false
        momentum_advection_interpolation = 'upwind'
      []
    []
    [ConservativeDiffuseInterfaceVOFSegregated]
      [vof]
        coupled_flow_physics = 'flow'
        passive_scalar_names = 'alpha'
        initial_scalar_variables = 'alpha_init'
        system_names = 'alpha_system'

        liquid_density_name = 'rho_w'
        gas_density_name = 'rho_o'
        liquid_dynamic_viscosity_name = 'mu_w'
        gas_dynamic_viscosity_name = 'mu_o'

        passive_scalar_advection_interpolation = 'upwind'
        compression_factor = 0.01
        interface_normal_functor = 'flow_interface_unit_normal_face'

        n_alpha_corrections = 1
        n_limiter_iterations = 6
      []
    []
    [ConservativeDiffuseInterfaceHeatTransferSegregated]
      [heat]
        coupled_vof_physics = 'vof'
        temperature_variable = 'T'
        liquid_fluid_properties = 'fp_water'
        gas_fluid_properties = 'fp_oil'
        system_names = 'energy_system'

        initial_temperature = '${T0}'
        liquid_thermal_expansion = '${beta_w}'
        gas_thermal_expansion = '${beta_o}'
        reference_temperature = '${T0}'

        energy_wall_types = 'fixed-temperature fixed-temperature heatflux heatflux'
        energy_wall_functors = '${fparse T0 + 0.5 * dT} ${fparse T0 - 0.5 * dT} 0 0'
      []
    []
  []
[]

[FluidProperties]
  [fp_water]
    type = TemperaturePressureFunctionFluidProperties
    rho = '${rho_w}'
    mu = '${mu_w}'
    k = '${k_w}'
    cp = '${cp_w}'
  []
  [fp_oil]
    type = TemperaturePressureFunctionFluidProperties
    rho = '${rho_o}'
    mu = '${mu_o}'
    k = '${k_o}'
    cp = '${cp_o}'
  []
[]

[FunctorMaterials]
  [constants]
    type = GenericFunctorMaterial
    prop_names = 'rho_w rho_o mu_w mu_o'
    prop_values = '${rho_w} ${rho_o} ${mu_w} ${mu_o}'
  []
  [alpha_interface]
    type = ParsedFunctorMaterial
    expression = 'alpha * (1 - alpha)'
    functor_names = 'alpha'
    property_name = 'alpha_interface_density'
  []
  # The temperature error in the cells outside the interface band, where the series conduction
  # profile is exact for an interface symmetric about y_i
  [pure_cell_error]
    type = ParsedFunctorMaterial
    expression = 'if(alpha * (1 - alpha) > 1e-6, 0, abs(T - T_exact))'
    functor_names = 'alpha T T_exact'
    property_name = 'pure_cell_T_error'
  []
[]

[Functions]
  [alpha_init]
    type = ParsedFunction
    expression = '0.5 * (1 - tanh((y - ${y_i}) / ${w}))'
  []
  # p_rgh = p + rho g y at rest, zero in the oil where the pressure is pinned: dp_rgh/dy =
  # g y drho/dy integrated down from the lid through the tanh profile, with the integral of alpha
  # in closed form. It reaches (rho_w - rho_o) g y_i in the water.
  [prgh_hydrostatic]
    type = ParsedFunction
    expression = 'g_drho * (y * a - 0.5 * ((y - ${H}) - ${w} * (log(cosh((y - ${y_i}) / ${w}))
                                                               - log(cosh((${H} - ${y_i}) / ${w})))))'
    symbol_names = 'g_drho a'
    symbol_values = '${fparse (rho_w - rho_o) * g} alpha_init'
  []
  [T_exact]
    type = ParsedFunction
    expression = 'if(y < ${y_i}, ${fparse T0 + 0.5 * dT} - ${q} * y / ${k_w},
                                 ${T_interface} - ${q} * (y - ${y_i}) / ${k_o})'
  []
[]

[Executioner]
  type = ReducedPressurePIMPLE
  rhie_chow_user_object = 'ins_rhie_chow_interpolator'

  momentum_systems = 'u_system v_system'
  pressure_system = 'pressure_system'
  active_scalar_systems = 'alpha_system'
  energy_system = 'energy_system'

  # The closed tub leaves the reduced pressure defined up to a constant; pin it in the oil
  pin_pressure = true
  pressure_pin_point = '${fparse 0.5 * W / nx} ${fparse H - 0.5 * H / ny} 0'
  pressure_pin_value = 0

  momentum_equation_relaxation = 0.9
  pressure_variable_relaxation = 0.7
  active_scalar_equation_relaxation = '1.0'
  energy_equation_relaxation = 1.0

  # At rest the normalized residuals compare round-off with a velocity that is round-off itself,
  # so they stay of order one; a fixed iteration count stands in for a tolerance here
  num_iterations = 5
  num_piso_iterations = 0
  continue_on_max_its = true
  print_fields = false

  momentum_absolute_tolerance = 1e-10
  pressure_absolute_tolerance = 1e-10
  active_scalar_absolute_tolerance = '1e-10'
  energy_absolute_tolerance = 1e-10

  momentum_l_abs_tol = 1e-12
  pressure_l_abs_tol = 1e-12
  active_scalar_l_abs_tol = 1e-12
  energy_l_abs_tol = 1e-12

  momentum_l_tol = 0
  pressure_l_tol = 0
  active_scalar_l_tol = 0
  energy_l_tol = 0

  momentum_petsc_options_iname = '-pc_type'
  momentum_petsc_options_value = 'lu'
  pressure_petsc_options_iname = '-pc_type'
  pressure_petsc_options_value = 'lu'
  active_scalar_petsc_options_iname = '-pc_type'
  active_scalar_petsc_options_value = 'lu'
  energy_petsc_options_iname = '-pc_type'
  energy_petsc_options_value = 'lu'

  dt = 0.01
  num_steps = 50
[]

[Postprocessors]
  [max_vx]
    type = ElementExtremeValue
    variable = 'vel_x'
    value_type = max
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [min_vx]
    type = ElementExtremeValue
    variable = 'vel_x'
    value_type = min
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [max_vy]
    type = ElementExtremeValue
    variable = 'vel_y'
    value_type = max
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [min_vy]
    type = ElementExtremeValue
    variable = 'vel_y'
    value_type = min
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [max_p]
    type = ElementExtremeValue
    variable = 'pressure'
    value_type = max
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [min_p]
    type = ElementExtremeValue
    variable = 'pressure'
    value_type = min
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [total_alpha]
    type = ElementIntegralVariablePostprocessor
    variable = 'alpha'
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [interface_measure]
    type = ElementIntegralFunctorPostprocessor
    functor = 'alpha_interface_density'
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [T_min]
    type = ElementExtremeValue
    variable = 'T'
    value_type = min
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [T_max]
    type = ElementExtremeValue
    variable = 'T'
    value_type = max
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [max_pure_cell_T_error]
    type = ElementExtremeFunctorValue
    functor = 'pure_cell_T_error'
    value_type = max
    execute_on = 'INITIAL TIMESTEP_END'
  []
  # Conductive heat flux leaving the fluid through the floor and the lid, per unit depth
  [floor_heat_flux]
    type = BoundaryLinearFVFluxIntegral
    linearfvkernels = 'heat_rho_h_diffusion'
    boundary = 'bottom'
    execute_on = 'TIMESTEP_END'
  []
  [lid_heat_flux]
    type = BoundaryLinearFVFluxIntegral
    linearfvkernels = 'heat_rho_h_diffusion'
    boundary = 'top'
    execute_on = 'TIMESTEP_END'
  []
[]

[Outputs]
  execute_on = 'INITIAL TIMESTEP_END'
  [csv]
    type = CSV
  []
[]
