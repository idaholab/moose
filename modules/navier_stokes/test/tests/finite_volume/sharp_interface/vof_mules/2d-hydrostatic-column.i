rho_l = 1000.0
rho_g = 1.0
mu_l = 1.0e-3
mu_g = 1.0e-5
g = 9.81

[Mesh]
  [mesh]
    type = CartesianMeshGenerator
    dim = 2
    dx = '0.5'
    dy = '1.0'
    ix = '10'
    iy = '20'
  []
[]

[Problem]
  linear_sys_names = 'u_system v_system pressure_system alpha_system'
  previous_nl_solution_required = true
[]

[Physics]
  [NavierStokes]
    [SharpInterfaceFlowSegregated]
      [flow]
        velocity_variable = 'vel_x vel_y'
        pressure_variable = 'pressure'

        compressibility = 'incompressible'
        density = 'rho_mixture'
        dynamic_viscosity = 'mu_mixture'
        gravity = '0 -${g} 0'
        volume_fraction_functor = 'alpha'
        surface_tension_coefficient = '0'
        create_curvature_producer = false

        initial_velocity = '0 0 0'
        initial_pressure = '0'

        wall_boundaries = 'left right top bottom'
        momentum_wall_types = 'noslip noslip noslip noslip'

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

        advected_interp_method = 'upwind'
        compression_factor = '0'
        interface_normal_functor = 'flow_interface_unit_normal_face'

        use_mules_correction = true
        n_alpha_corrections = 2
        n_limiter_iterations = 4
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
    expression = 'if(y < 0.5, 1, 0)'
  []
[]

[AuxVariables]
  [pressure_head]
    type = MooseVariableFVReal
  []
  [rho_gh]
    type = MooseVariableFVReal
  []
  [total_pressure]
    type = MooseVariableFVReal
  []
[]

[AuxKernels]
  [pressure_head]
    type = FunctorAux
    variable = pressure_head
    functor = 'flow_reduced_pressure_head'
    execute_on = 'TIMESTEP_END'
  []
  [rho_gh]
    type = ParsedAux
    variable = rho_gh
    expression = 'rho * gh'
    functor_names = 'rho_mixture flow_reduced_pressure_head'
    functor_symbols = 'rho gh'
    evaluate_functors_on_qp = false
    execute_on = 'TIMESTEP_END'
  []
  [total_pressure]
    type = ParsedAux
    variable = total_pressure
    expression = 'pressure + rho_gh'
    coupled_variables = 'pressure rho_gh'
    execute_on = 'TIMESTEP_END'
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
  num_piso_iterations = 1
  continue_on_max_its = true
  print_fields = false

  momentum_absolute_tolerance = 1e-12
  pressure_absolute_tolerance = 1e-12
  volume_fraction_absolute_tolerance = '1e-12'

  momentum_l_abs_tol = 1e-14
  pressure_l_abs_tol = 1e-14
  volume_fraction_l_abs_tol = 1e-14

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
  pressure_pin_point = '0.0 0.0 0.0'
  pressure_pin_value = 0.0

  startup_pressure_initialization = 'equilibrium-seed'
  startup_flux_corrections = 2

  volume_fraction_subcycles = 1
  dt = 0.01
  num_steps = 3
[]

[Postprocessors]
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
  [pressure_bottom]
    type = PointValue
    variable = 'pressure'
    point = '0.25 0.25 0'
  []
  [pressure_top]
    type = PointValue
    variable = 'pressure'
    point = '0.25 0.75 0'
  []
  [total_pressure_bottom]
    type = PointValue
    variable = 'total_pressure'
    point = '0.25 0.25 0'
  []
  [total_pressure_interface_liquid]
    type = PointValue
    variable = 'total_pressure'
    point = '0.25 0.475 0'
  []
  [total_pressure_interface_gas]
    type = PointValue
    variable = 'total_pressure'
    point = '0.25 0.525 0'
  []
  [total_pressure_top]
    type = PointValue
    variable = 'total_pressure'
    point = '0.25 0.75 0'
  []
  [total_pressure_interface_jump]
    type = ParsedPostprocessor
    expression = 'abs(total_pressure_interface_liquid-total_pressure_interface_gas)'
    pp_names = 'total_pressure_interface_liquid total_pressure_interface_gas'
  []
[]

[Outputs]
  execute_on = 'TIMESTEP_END'
  csv = true
[]
