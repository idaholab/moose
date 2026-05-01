mu_l = 0.01
mu_g = 0.01
rho_l = 1.0
rho_g = 1.0
u_in = 1.0

[Mesh]
  [mesh]
    type = CartesianMeshGenerator
    dim = 1
    dx = '1'
    ix = '20'
  []
[]

[Problem]
  linear_sys_names = 'u_system pressure_system alpha_system'
  previous_nl_solution_required = true
[]

[Physics]
  [NavierStokes]
    [SharpInterfaceFlowSegregated]
      [flow]
        velocity_variable = 'vel_x'
        pressure_variable = 'pressure'

        compressibility = 'incompressible'
        density = 'rho_mixture'
        dynamic_viscosity = 'mu_mixture'
        gravity = '0 0 0'
        volume_fraction_functor = 'alpha'
        surface_tension_coefficient = '0'
        create_curvature_producer = false

        initial_velocity = '${u_in} 0 0'
        initial_pressure = '0'

        inlet_boundaries = 'left'
        momentum_inlet_types = 'fixed-velocity'
        momentum_inlet_functors = '${u_in}'

        outlet_boundaries = 'right'
        momentum_outlet_types = 'fixed-pressure'
        pressure_functors = '0'

        orthogonality_correction = false
        momentum_two_term_bc_expansion = false
        pressure_two_term_bc_expansion = false
        momentum_advection_interpolation = 'upwind'
      []
    []
    [SharpInterfaceVOFSegregated]
      [vof]
        coupled_flow_physics = 'flow'
        volume_fraction_variable = 'alpha'
        initial_volume_fraction = 'alpha_init'
        system_names = 'alpha_system'

        liquid_density_name = 'rho_l'
        gas_density_name = 'rho_g'
        liquid_dynamic_viscosity_name = 'mu_l'
        gas_dynamic_viscosity_name = 'mu_g'

        volume_fraction_inlet_functors = '0'
        volume_fraction_two_term_bc_expansion = true
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
    expression = 'if(x < 0.7, 1, 0) - if(x < 0.4, 1, 0)'
  []
[]

[Executioner]
  type = ReducedPressurePIMPLE
  rhie_chow_user_object = 'ins_rhie_chow_interpolator'

  momentum_systems = 'u_system'
  pressure_system = 'pressure_system'
  volume_fraction_systems = 'alpha_system'

  momentum_equation_relaxation = 0.7
  pressure_variable_relaxation = 0.3
  volume_fraction_equation_relaxation = '1.0'

  num_iterations = 50
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
  pressure_pin_point = '1.0 0.0 0.0'

  volume_fraction_subcycles = 3
  dt = 0.005
  num_steps = 80
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
  [vel_x_average]
    type = ElementAverageValue
    variable = 'vel_x'
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
[]

[Outputs]
  execute_on = 'TIMESTEP_END'
  csv = true
  exodus = false
[]
