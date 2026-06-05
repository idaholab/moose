rho_l = 10.0
rho_g = 1.0
mu_l = 1.0e-3
mu_g = 1.0e-3

[Mesh]
  [mesh]
    type = CartesianMeshGenerator
    dim = 2
    dx = '1.25'
    dy = '0.4'
    ix = '5'
    iy = '3'
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
        gravity = '0 0 0'
        volume_fraction_functor = 'alpha'
        surface_tension_coefficient = '0'
        create_curvature_producer = false

        initial_velocity = 'u_init v_init 0'
        initial_pressure = '0'

        wall_boundaries = 'left right bottom'
        momentum_wall_types = 'noslip noslip noslip'

        outlet_boundaries = 'top'
        momentum_outlet_types = 'fixed-pressure'
        pressure_functors = '0'

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
    expression = 'if(x < 0.5 & y < 0.25, 1, 0)'
  []
  [u_init]
    type = ParsedFunction
    expression = '0.1*sin(pi*x/1.25)*sin(pi*y/0.4)'
  []
  [v_init]
    type = ParsedFunction
    expression = '0.05*sin(pi*x/1.25)*sin(pi*y/0.4)'
  []
[]

[Executioner]
  type = ReducedPressurePIMPLE
  rhie_chow_user_object = 'ins_rhie_chow_interpolator'

  momentum_systems = 'u_system v_system'
  pressure_system = 'pressure_system'
  volume_fraction_systems = 'alpha_system'

  should_solve_momentum = false
  should_solve_volume_fractions = false

  momentum_equation_relaxation = 1.0
  pressure_variable_relaxation = 1.0
  volume_fraction_equation_relaxation = '1.0'

  num_iterations = 1
  num_piso_iterations = 0
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

  startup_pressure_initialization = 'none'

  dump_pressure_outer_debug_csv = true
  dump_pressure_outer_debug_start_timestep = 1
  dump_pressure_outer_debug_end_timestep = 1
  dump_pressure_outer_debug_max_outer_iterations = 1
  print_pressure_hbya_parity_summary = true

  dt = 1e-3
  num_steps = 1
[]

[Postprocessors]
  [geometric_reconstruction_error]
    type = SharpInterfacePressureWritebackReconstructionError
    constant_vector = '1.25 -0.75 0'
  []
  [cell_divergence_l2]
    type = RhieChowCellContinuityResidual
    rhie_chow_user_object = 'ins_rhie_chow_interpolator'
    metric = l2
  []
  [cell_divergence_max]
    type = RhieChowCellContinuityResidual
    rhie_chow_user_object = 'ins_rhie_chow_interpolator'
    metric = max_abs
  []
  [face_flux_consistency_l2]
    type = RhieChowFaceFluxConsistencyError
    rhie_chow_user_object = 'ins_rhie_chow_interpolator'
    quantity = l2
  []
  [correction_branch_consistency]
    type = ConservativeSharpInterfaceFluxBranchConsistencyError
    rhie_chow_user_object = 'ins_rhie_chow_interpolator'
    quantity = pressure_correction
    metric = l2
  []
  [total_branch_consistency]
    type = ConservativeSharpInterfaceFluxBranchConsistencyError
    rhie_chow_user_object = 'ins_rhie_chow_interpolator'
    quantity = total
    metric = l2
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
[]

[Outputs]
  execute_on = 'TIMESTEP_END'
  csv = true
[]
