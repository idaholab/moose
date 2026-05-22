rho_l = 10.0
rho_g = 1.0
mu_l = 1.0e-3
mu_g = 1.0e-3
g = 9.81

[Mesh]
  [mesh]
    type = CartesianMeshGenerator
    dim = 2
    dx = '1.6'
    dy = '0.8'
    ix = '32'
    iy = '16'
  []
[]

[Problem]
  linear_sys_names = 'u_system v_system pressure_system alpha_system'
  previous_nl_solution_required = true
[]

[Physics]
  [NavierStokes]
    [ConservativeSharpInterfaceFlowSegregated]
      [flow]
        velocity_variable = 'vel_x vel_y'
        pressure_variable = 'pressure'

        compressibility = 'incompressible'
        density = 'rho_mixture'
        dynamic_viscosity = 'mu_mixture'
        gravity = '0 -${g} 0'
        volume_fraction_functor = 'alpha'
        reference_pressure_point = '0 0.8 0'
        surface_tension_coefficient = '0'
        create_curvature_producer = false

        initial_velocity = '0 0 0'
        initial_pressure = 'pressure_init'

        wall_boundaries = 'left right bottom'
        momentum_wall_types = 'noslip noslip noslip'

        outlet_boundaries = 'top'
        momentum_outlet_types = 'fixed-pressure'
        pressure_functors = '0'

        orthogonality_correction = false
        momentum_two_term_bc_expansion = false
        pressure_two_term_bc_expansion = false
        momentum_advection_interpolation = 'upwind'
      []
    []
    [ConservativeSharpInterfaceVOFSegregated]
      [vof]
        coupled_flow_physics = 'flow'
        volume_fraction_variable = 'alpha'
        initial_volume_fraction = 'alpha_init'
        system_names = 'alpha_system'
        volume_fraction_outlet_type = 'inlet-outlet'

        liquid_density_name = 'rho_l'
        gas_density_name = 'rho_g'
        liquid_dynamic_viscosity_name = 'mu_l'
        gas_dynamic_viscosity_name = 'mu_g'

        advected_interp_method = 'upwind'
        compression_factor = '2'
        interface_normal_functor = 'flow_interface_unit_normal_face'

        use_mules_correction = true
        n_alpha_corrections = 3
        n_limiter_iterations = 6
      []
    []
  []
[]

[FunctorMaterials]
  [constants]
    type = GenericFunctorMaterial
    prop_names = 'rho_l rho_g mu_l mu_g'
    prop_values = '${rho_l} ${rho_g} ${mu_l} ${mu_g}'
  []
[]

[Functions]
  [alpha_init]
    type = ParsedFunction
    expression = 'if(x < 0.4 & y < 0.6, 1, 0)'
  []
  [pressure_init]
    type = ParsedFunction
    expression = 'if(x < 0.4 & y < 0.6, -(${rho_l}-${rho_g})*${g}*0.2, 0)'
  []
[]

[Executioner]
  type = ReducedPressurePIMPLE
  rhie_chow_user_object = 'ins_rhie_chow_interpolator'

  momentum_systems = 'u_system v_system'
  pressure_system = 'pressure_system'
  volume_fraction_systems = 'alpha_system'

  momentum_equation_relaxation = 0.7
  pressure_variable_relaxation = 0.3
  volume_fraction_equation_relaxation = '1.0'

  num_iterations = 1
  num_piso_iterations = 0
  continue_on_max_its = true
  print_fields = false

  momentum_absolute_tolerance = 1e-10
  pressure_absolute_tolerance = 1e-10
  volume_fraction_absolute_tolerance = '1e-10'

  momentum_l_abs_tol = 1e-12
  pressure_l_abs_tol = 1e-12
  volume_fraction_l_abs_tol = 1e-12

  momentum_l_tol = 0
  pressure_l_tol = 0
  volume_fraction_l_tol = 0

  momentum_petsc_options_iname = '-pc_type'
  momentum_petsc_options_value = 'lu'
  pressure_petsc_options_iname = '-pc_type'
  pressure_petsc_options_value = 'lu'
  volume_fraction_petsc_options_iname = '-pc_type'
  volume_fraction_petsc_options_value = 'lu'

  pin_pressure = true
  pressure_pin_point = '0.0 0.8 0.0'

  startup_pressure_initialization = 'projection-only'
  startup_flux_corrections = 2

  volume_fraction_subcycles = 3
  dt = 0.0002
  num_steps = 100
[]

[Postprocessors]
  [alpha_average]
    type = ElementAverageValue
    variable = 'alpha'
  []
  [alpha_min]
    type = ElementExtremeValue
    variable = 'alpha'
    value_type = min
  []
  [alpha_max]
    type = ElementExtremeValue
    variable = 'alpha'
    value_type = max
  []
  [vel_x_min]
    type = ElementExtremeValue
    variable = 'vel_x'
    value_type = min
  []
  [vel_x_max]
    type = ElementExtremeValue
    variable = 'vel_x'
    value_type = max
  []
  [vel_y_min]
    type = ElementExtremeValue
    variable = 'vel_y'
    value_type = min
  []
  [vel_y_max]
    type = ElementExtremeValue
    variable = 'vel_y'
    value_type = max
  []
  [alpha_front_near]
    type = PointValue
    variable = 'alpha'
    point = '0.45 0.05 0'
  []
  [alpha_front_mid]
    type = PointValue
    variable = 'alpha'
    point = '0.75 0.05 0'
  []
  [alpha_front_far]
    type = PointValue
    variable = 'alpha'
    point = '1.05 0.05 0'
  []
  [alpha_column_top]
    type = PointValue
    variable = 'alpha'
    point = '0.20 0.55 0'
  []
[]

[Outputs]
  execute_on = 'TIMESTEP_END'
  csv = true
  exodus = false
[]
