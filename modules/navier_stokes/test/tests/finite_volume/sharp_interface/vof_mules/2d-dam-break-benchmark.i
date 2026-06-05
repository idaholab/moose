# Benchmark-style sharp-interface VOF dam break based on the Martin & Moyce (1952)
# reference case used in:
#   /Users/chowr/Work/projects/NS_Peter/moose_repo/moose/modules/navier_stokes/examples/two-phase
# Reference experimental curves:
#   reference_data/dambreak_height.csv
#   reference_data/Dambreak_length.csv

rho_l = 998.19
rho_g = 1.185
mu_l = 1.0e-3
mu_g = 1.48e-5
g = 9.81

initial_length = 0.05715
aspect_ratio = 1.0
downstream_domain_lengths = 7.0
vertical_domain_factor = 1.25
nx = 280
ny = 50
dam_x = ${initial_length}
dam_y = ${fparse aspect_ratio * initial_length}
domain_dims_x = ${fparse downstream_domain_lengths * initial_length}
domain_dims_y = ${fparse vertical_domain_factor * dam_y}
cell_dx = ${fparse domain_dims_x / nx}

c_alpha = 0.01

[Mesh]
  [mesh]
    type = CartesianMeshGenerator
    dim = 2
    dx = '${domain_dims_x}'
    dy = '${domain_dims_y}'
    ix = '${nx}'
    iy = '${ny}'
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
        surface_tension_coefficient = '0'
        create_curvature_producer = true
        curvature_delta_n_mode = 'fixed'
        curvature_delta_n_fixed_value = 7.88307825360258e-07
        n_alpha_smooth_curvature = 0

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
        compression_factor = '${c_alpha}'
        interface_normal_functor = 'flow_interface_unit_normal_face'

        use_mules_correction = true
        alpha_apply_prev_corr = false
        n_alpha_corrections = 2
        n_limiter_iterations = 3
        use_cell_summed_mules_limiter = true
        use_local_mules_bounds = true
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
    expression = 'if(x < ${dam_x} & y < ${dam_y}, 1, 0)'
  []
  [pressure_init]
    type = ParsedFunction
    expression = 'if(x < ${dam_x} & y < ${dam_y}, -(${rho_l}-${rho_g})*${g}*(${domain_dims_y}-${dam_y}), 0)'
  []
[]

[AuxVariables]
  [water_heights]
    type = MooseVariableFVReal
  []
  [water_lengths]
    type = MooseVariableFVReal
  []
[]

[AuxKernels]
  [water_heights]
    type = ParsedAux
    variable = water_heights
    coupled_variables = 'alpha'
    expression = 'if(alpha > 0.5, y, 0)'
    use_xyzt = true
    execute_on = 'TIMESTEP_END'
  []
  [water_lengths]
    type = ParsedAux
    variable = water_lengths
    coupled_variables = 'alpha'
    expression = 'if(alpha > 0.5, x, 0)'
    use_xyzt = true
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

  should_solve_momentum = false
  num_iterations = 1
  num_piso_iterations = 1
  num_pressure_nonorthogonal_correctors = 0
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

  startup_pressure_initialization = 'projection-only'
  startup_flux_corrections = 2

  volume_fraction_subcycles = 2
  dt = 1.0e-4
  end_time = 0.4
[]

[Postprocessors]
  [compute_front_height]
    type = SubcellInterfacialTipPosition
    volume_fraction = alpha
    tip_direction = x
    reported_component = y
    extremum_type = max
    reported_extremum_type = max
    tip_band_width = '${fparse 2.0 * domain_dims_x / 200.0}'
    threshold = 0.5
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [compute_front_length]
    type = SubcellInterfacialTipPosition
    volume_fraction = alpha
    tip_direction = x
    reported_component = x
    extremum_type = max
    threshold = 0.5
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [compute_front_length_subcell]
    type = SubcellInterfacialTipPosition
    volume_fraction = alpha
    tip_direction = x
    reported_component = x
    extremum_type = max
    threshold = 0.5
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [compute_front_height_subcell]
    type = SubcellInterfacialTipPosition
    volume_fraction = alpha
    tip_direction = x
    reported_component = y
    extremum_type = max
    reported_extremum_type = max
    tip_band_width = '${fparse 2.0 * domain_dims_x / 200.0}'
    threshold = 0.5
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [compute_front_length_over_initial]
    type = ParsedPostprocessor
    expression = 'x / ${dam_x}'
    pp_names = 'compute_front_length_subcell'
    pp_symbols = 'x'
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [compute_back_column_height]
    type = SubcellInterfacialPosition
    volume_fraction = alpha
    direction = y
    extremum_type = max
    threshold = 0.5
    secondary_min = 0
    secondary_max = '${fparse 2.5 * cell_dx}'
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [compute_back_column_height_over_initial]
    type = ParsedPostprocessor
    expression = 'h / ${dam_y}'
    pp_names = 'compute_back_column_height'
    pp_symbols = 'h'
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [nondimensional_time_tau]
    type = ParsedPostprocessor
    expression = 't * sqrt(${g} / ${dam_x})'
    use_t = true
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [total_alpha]
    type = ElementIntegralVariablePostprocessor
    variable = 'alpha'
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [liquid_com_x]
    type = LiquidCenterOfMass
    volume_fraction = alpha
    liquid_density = rho_l
    direction = x
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [liquid_com_y]
    type = LiquidCenterOfMass
    volume_fraction = alpha
    liquid_density = rho_l
    direction = y
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [liquid_horizontal_momentum]
    type = LiquidMomentum
    volume_fraction = alpha
    liquid_density = rho_l
    velocity = vel_x
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
  [max_u]
    type = ElementExtremeValue
    variable = 'vel_x'
    value_type = max
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [max_v]
    type = ElementExtremeValue
    variable = 'vel_y'
    value_type = max
    execute_on = 'INITIAL TIMESTEP_END'
  []
[]

[Outputs]
  execute_on = 'TIMESTEP_END'
  csv = true
  exodus = true
[]
