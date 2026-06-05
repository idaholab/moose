# Martin-Moyce rectangular dam-break case matched to digitized data that exist
# for both surge-front position and top-of-column height:
#   n^2 = 1, a = 2.25 in.
#
# In Martin-Moyce notation:
#   full base width = 2 a = 4.5 in
#   initial height  = n^2 a = 2.25 in

rho_l = 998.19
rho_g = 1.185
mu_l = 1.0e-3
mu_g = 1.48e-5
g = 9.81

a_length = 0.05715
n_squared_value = 1.0
n_value = 1.0

dam_x = ${fparse 2.0 * a_length}
dam_y = ${fparse n_squared_value * a_length}

# Keep roughly the same absolute resolution as the baseline 200x50 benchmark.
domain_dims_x = ${fparse 6.0 * a_length}
domain_dims_y = ${fparse 1.25 * dam_y}
cell_dx = ${fparse domain_dims_x / 240.0}
cell_dy = ${fparse domain_dims_y / 50.0}

c_alpha = 0.01

[Mesh]
  [mesh]
    type = CartesianMeshGenerator
    dim = 2
    dx = '${domain_dims_x}'
    dy = '${domain_dims_y}'
    ix = '240'
    iy = '50'
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
        velocity_variable = 'rhou rhov'
        pressure_variable = 'pressure'

        compressibility = 'incompressible'
        density = 'rho_mixture'
        dynamic_viscosity = 'mu_mixture'
        gravity = '0 -${g} 0'
        volume_fraction_functor = 'alpha'
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
        compression_factor = '${c_alpha}'
        interface_normal_functor = 'flow_interface_unit_normal_face'

        use_mules_correction = true
        alpha_apply_prev_corr = false
        n_alpha_corrections = 1
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
    expression = 'if(x < ${dam_x} & y < ${dam_y}, 1, 0)'
  []
  [pressure_init]
    type = ParsedFunction
    expression = 'if(x < ${dam_x} & y < ${dam_y}, -(${rho_l}-${rho_g})*${g}*(${domain_dims_y}-${dam_y}), 0)'
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

  startup_pressure_initialization = 'projection-only'
  startup_flux_corrections = 2

  volume_fraction_subcycles = 2
  dt = 1.0e-4
  end_time = 0.5
[]

[Postprocessors]
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
  [paper_front_position_x_raw]
    type = SubcellInterfacialPosition
    volume_fraction = alpha
    direction = x
    extremum_type = max
    threshold = 0.5
    secondary_min = 0
    secondary_max = '${fparse 2.5 * cell_dy}'
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [paper_top_height_y_raw]
    type = SubcellInterfacialPosition
    volume_fraction = alpha
    direction = y
    extremum_type = max
    threshold = 0.5
    secondary_min = 0
    secondary_max = '${fparse 2.5 * cell_dx}'
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [paper_front_Z]
    type = ParsedPostprocessor
    expression = '(x - ${a_length}) / ${a_length}'
    pp_names = 'paper_front_position_x_raw'
    pp_symbols = 'x'
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [paper_top_H]
    type = ParsedPostprocessor
    expression = 'h / (${a_length} * ${n_squared_value})'
    pp_names = 'paper_top_height_y_raw'
    pp_symbols = 'h'
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [paper_T]
    type = ParsedPostprocessor
    expression = '${n_value} * t * sqrt(${g} / ${a_length})'
    use_t = true
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [paper_tau]
    type = ParsedPostprocessor
    expression = 't * sqrt(${g} / ${a_length})'
    use_t = true
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [total_alpha]
    type = ElementIntegralVariablePostprocessor
    variable = 'alpha'
    execute_on = 'INITIAL TIMESTEP_END'
  []
[]

[Outputs]
  execute_on = 'TIMESTEP_END'
  csv = true
  exodus = false
  file_base = '2d-dam-break-benchmark-paper-n1-a2p25'
[]
