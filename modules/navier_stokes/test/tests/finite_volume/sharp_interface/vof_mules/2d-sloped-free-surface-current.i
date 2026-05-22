rho_l = 998.19
rho_g = 1.185
mu_l = 1.0e-3
mu_g = 1.48e-5
g = 9.81

domain_x = 1.0
domain_y = 0.5
nx = 64
ny = 32

h0 = 0.20
eta = 0.03
pi_const = 3.141592653589793

cell_dx = ${fparse domain_x / nx}
left_probe_x = ${fparse 0.20 * domain_x}
center_probe_x = ${fparse 0.50 * domain_x}
right_probe_x = ${fparse 0.80 * domain_x}
probe_half_width = ${fparse 1.5 * cell_dx}

[Mesh]
  [mesh]
    type = CartesianMeshGenerator
    dim = 2
    dx = '${domain_x}'
    dy = '${domain_y}'
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
        reference_pressure_point = '0 ${domain_y} 0'
        surface_tension_coefficient = '0'
        create_curvature_producer = false

        initial_velocity = 'u_init v_init 0'
        initial_pressure = 'pressure_init'

        wall_boundaries = 'left right top bottom'
        momentum_wall_types = 'noslip noslip noslip noslip'

        orthogonality_correction = false
        momentum_two_term_bc_expansion = false
        pressure_two_term_bc_expansion = false
        momentum_advection_interpolation = 'average'
      []
    []
    [ConservativeSharpInterfaceVOFSegregated]
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
        alpha_apply_prev_corr = false
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
  [free_surface_height]
    type = ParsedFunction
    expression = '${h0} + ${eta}*cos(${pi_const}*x/${domain_x})'
  []
  [alpha_init]
    type = ParsedFunction
    expression = 'if(y < ${h0} + ${eta}*cos(${pi_const}*x/${domain_x}), 1, 0)'
  []
  [pressure_init]
    type = ParsedFunction
    expression = 'if(y < ${h0} + ${eta}*cos(${pi_const}*x/${domain_x}), -(${rho_l}-${rho_g})*${g}*(${domain_y}-(${h0} + ${eta}*cos(${pi_const}*x/${domain_x}))), 0)'
  []
  [u_init]
    type = ParsedFunction
    expression = '0.02*sin(${pi_const}*x/${domain_x})*sin(${pi_const}*y/${domain_y})'
  []
  [v_init]
    type = ParsedFunction
    expression = '0.01*sin(${pi_const}*x/${domain_x})*sin(${pi_const}*y/${domain_y})'
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

  num_iterations = 2
  num_piso_iterations = 1
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
  pressure_pin_point = '0.0 ${domain_y} 0.0'
  pressure_pin_value = 0.0

  startup_pressure_initialization = 'projection-only'
  startup_flux_corrections = 2

  volume_fraction_subcycles = 1
  dt = 1.0e-4
  num_steps = 50
[]

[Postprocessors]
  [total_alpha]
    type = ElementIntegralVariablePostprocessor
    variable = 'alpha'
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [cell_divergence_l2]
    type = RhieChowCellContinuityResidual
    rhie_chow_user_object = 'ins_rhie_chow_interpolator'
    metric = l2
    execute_on = 'TIMESTEP_END'
  []
  [cell_divergence_max]
    type = RhieChowCellContinuityResidual
    rhie_chow_user_object = 'ins_rhie_chow_interpolator'
    metric = max_abs
    execute_on = 'TIMESTEP_END'
  []
  [face_flux_consistency_internal]
    type = RhieChowFaceFluxConsistencyError
    rhie_chow_user_object = 'ins_rhie_chow_interpolator'
    quantity = internal_l2
    execute_on = 'TIMESTEP_END'
  []
  [correction_branch_consistency]
    type = ConservativeSharpInterfaceFluxBranchConsistencyError
    rhie_chow_user_object = 'ins_rhie_chow_interpolator'
    quantity = pressure_correction
    metric = l2
    execute_on = 'TIMESTEP_END'
  []
  [liquid_com_x]
    type = LiquidCenterOfMass
    volume_fraction = alpha
    liquid_density = rho_l
    direction = x
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [liquid_horizontal_momentum]
    type = LiquidMomentum
    volume_fraction = alpha
    liquid_density = rho_l
    velocity = vel_x
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [interface_height_left]
    type = SubcellInterfacialPosition
    volume_fraction = alpha
    direction = y
    extremum_type = max
    threshold = 0.5
    secondary_min = '${fparse left_probe_x - probe_half_width}'
    secondary_max = '${fparse left_probe_x + probe_half_width}'
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [interface_height_center]
    type = SubcellInterfacialPosition
    volume_fraction = alpha
    direction = y
    extremum_type = max
    threshold = 0.5
    secondary_min = '${fparse center_probe_x - probe_half_width}'
    secondary_max = '${fparse center_probe_x + probe_half_width}'
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [interface_height_right]
    type = SubcellInterfacialPosition
    volume_fraction = alpha
    direction = y
    extremum_type = max
    threshold = 0.5
    secondary_min = '${fparse right_probe_x - probe_half_width}'
    secondary_max = '${fparse right_probe_x + probe_half_width}'
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
  exodus = false
[]
