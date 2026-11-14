# Conserved-enthalpy transport at uniform temperature. This is the volume-fraction slab of
# vof_advection.i with the rho*h equation solved alongside it. Everything starts at T0 and only T0
# enters, so the exact solution keeps T = T0 everywhere while rho*h jumps across the moving
# interface. The phase enthalpies are nonlinear in T and differ strongly at T0, so T stays at T0
# only if the rho*h face flux is built from the same limited volume-fraction flux as alpha
# (accumulated over the alpha subcycles) and the per-cell recovery of T inverts h_k(T) exactly.

rho_l = 998.19
rho_g = 998.19
mu_l = 1.0e-3
mu_g = 1.0e-3
g = 0

domain_dims_x = 1.0
domain_dims_y = 0.04

c_alpha = 0.0
u_inlet = 1.0

T0 = 300
# The reference enthalpies differ by 1e5 J/kg so that rho*h jumps by about 1e8 J/m^3 across the
# interface; a flux inconsistent with the alpha flux then shows up as a large drift in T.
T_ref = 250
e_ref_l = 1e5
e_ref_g = 2e5

[Mesh]
  [mesh]
    type = CartesianMeshGenerator
    dim = 2
    dx = '${domain_dims_x}'
    dy = '${domain_dims_y}'
    ix = '50'
    iy = '2'
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
        volume_fraction_functor = 'alpha'

        initial_velocity = '${u_inlet} 0 0'
        initial_pressure = 'pressure_init'

        inlet_boundaries = 'left'
        momentum_inlet_types = 'fixed-velocity'
        momentum_inlet_functors = '${u_inlet} 0'

        wall_boundaries = 'bottom top'
        momentum_wall_types = 'symmetry symmetry'

        outlet_boundaries = 'right'
        momentum_outlet_types = 'fixed-pressure'
        pressure_functors = '0'

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
        passive_scalar_inlet_types = 'fixed-value'
        passive_scalar_inlet_functors = '0'
        initial_scalar_variables = 'alpha_init'
        system_names = 'alpha_system'

        liquid_density_name = 'rho_l'
        gas_density_name = 'rho_g'
        liquid_dynamic_viscosity_name = 'mu_l'
        gas_dynamic_viscosity_name = 'mu_g'

        passive_scalar_advection_interpolation = 'upwind'
        compression_factor = '${c_alpha}'
        interface_normal_functor = 'flow_interface_unit_normal_face'

        n_alpha_corrections = 1
        n_limiter_iterations = 6
      []
    []
    [ConservativeDiffuseInterfaceHeatTransferSegregated]
      [heat]
        coupled_vof_physics = 'vof'
        conserved_enthalpy_variable = 'rho_h'
        temperature_variable = 'T'
        liquid_fluid_properties = 'fp_l'
        gas_fluid_properties = 'fp_g'
        system_names = 'energy_system'

        initial_temperature = '${T0}'

        energy_inlet_types = 'fixed-temperature'
        energy_inlet_functors = '${T0}'
        energy_wall_types = 'heatflux heatflux'
        energy_wall_functors = '0 0'
      []
    []
  []
[]

[FluidProperties]
  # Constant density matching the volume-fraction physics, so h = e(T) + p/rho has
  # dh/dT = cp(T). The linear cp(T) makes h_k(T) quadratic, which exercises the Newton recovery
  # of T; with constant cp the recovery would be a linear solve.
  [fp_l]
    type = TemperaturePressureFunctionFluidProperties
    rho = '${rho_l}'
    mu = '${mu_l}'
    k = 0.6
    cp = cp_l
    T_ref = '${T_ref}'
    e_ref = '${e_ref_l}'
  []
  [fp_g]
    type = TemperaturePressureFunctionFluidProperties
    rho = '${rho_g}'
    mu = '${mu_g}'
    k = 0.026
    cp = cp_g
    T_ref = '${T_ref}'
    e_ref = '${e_ref_g}'
  []
[]

[FunctorMaterials]
  [alpha_x]
    type = ParsedFunctorMaterial
    expression = 'alpha * x'
    functor_names = 'alpha'
    property_name = 'alpha_x_density'
  []
  [alpha_interface]
    type = ParsedFunctorMaterial
    expression = 'alpha * (1 - alpha)'
    functor_names = 'alpha'
    property_name = 'alpha_interface_density'
  []
  [constants]
    type = GenericFunctorMaterial
    prop_names = 'rho_l rho_g mu_l mu_g'
    prop_values = '${rho_l} ${rho_g} ${mu_l} ${mu_g}'
  []
[]

[Functions]
  [alpha_init]
    type = ParsedFunction
    expression = 'if(x > 0.1 & x < 0.3, 1, 0)'
  []
  [pressure_init]
    type = ParsedFunction
    expression = '0'
  []
  # TemperaturePressureFunctionFluidProperties evaluates its functions at (x, y) = (T, p)
  [cp_l]
    type = ParsedFunction
    expression = '4000 + 10 * x'
  []
  [cp_g]
    type = ParsedFunction
    expression = '1000 + 2 * x'
  []
[]

[Executioner]
  type = ReducedPressurePIMPLE
  rhie_chow_user_object = 'ins_rhie_chow_interpolator'

  momentum_systems = 'u_system v_system'
  pressure_system = 'pressure_system'
  active_scalar_systems = 'alpha_system'
  energy_system = 'energy_system'

  momentum_equation_relaxation = 0.7
  pressure_variable_relaxation = 0.3
  active_scalar_equation_relaxation = '1.0'
  energy_equation_relaxation = 1.0

  num_iterations = 1
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

  startup_flux_corrections = 2

  volume_fraction_subcycles = 2
  dt = 4.0e-3
  num_steps = 25
[]

[Postprocessors]
  [alpha_x_moment]
    type = ElementIntegralFunctorPostprocessor
    functor = 'alpha_x_density'
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [interface_measure]
    type = ElementIntegralFunctorPostprocessor
    functor = 'alpha_interface_density'
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [max_vx]
    type = ElementExtremeValue
    variable = 'vel_x'
    value_type = max
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
  [alpha_min]
    type = ElementExtremeValue
    variable = 'alpha'
    value_type = min
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [alpha_max]
    type = ElementExtremeValue
    variable = 'alpha'
    value_type = max
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [total_alpha]
    type = ElementIntegralVariablePostprocessor
    variable = 'alpha'
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
  # Gas at T0 enters and leaves and the slab never reaches the outlet, so the total is constant
  [total_rho_h]
    type = ElementIntegralVariablePostprocessor
    variable = 'rho_h'
    execute_on = 'INITIAL TIMESTEP_END'
  []
[]

[Outputs]
  execute_on = 'INITIAL TIMESTEP_END'
  [csv]
    type = CSV
  []
[]
