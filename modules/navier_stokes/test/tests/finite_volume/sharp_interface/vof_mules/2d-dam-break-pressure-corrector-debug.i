# Pressure-corrector debug input for the sharp-interface dam-break benchmark.
# Freezes the initial interface, skips the momentum predictor, and performs a
# single pressure-only outer iteration so the reduced-pressure writeback path
# can be audited in isolation on the hydrostatic initial condition.

rho_l = 998.19
rho_g = 1.185
mu_l = 1.0e-3
mu_g = 1.48e-5
g = 9.81

initial_length = 0.05715
domain_dims_x = ${fparse 5.0 * initial_length}
domain_dims_y = ${fparse 1.25 * initial_length}
dam_x = ${initial_length}
dam_y = ${initial_length}
cut_y = ${fparse 0.5 * dam_y}

c_alpha = 0.01

[Mesh]
  [mesh]
    type = CartesianMeshGenerator
    dim = 2
    dx = '${domain_dims_x}'
    dy = '${domain_dims_y}'
    ix = '200'
    iy = '50'
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
        reference_pressure_point = '0 ${domain_dims_y} 0'
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
        compression_factor = '${c_alpha}'
        interface_normal_functor = 'flow_interface_unit_normal_face'

        use_mules_correction = true
        alpha_apply_prev_corr = true
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
    expression = 'if(x < ${dam_x} & y < ${dam_y}, 1, 0)'
  []
  [pressure_init]
    type = ParsedFunction
    expression = 'if(x < ${dam_x} & y < ${dam_y}, -(${rho_l}-${rho_g})*${g}*(${domain_dims_y}-${dam_y}), 0)'
  []
[]

[AuxVariables]
  [total_pressure_audit]
    type = MooseVariableFVReal
  []
  [analytic_reduced_pressure]
    type = MooseVariableFVReal
  []
  [analytic_total_pressure]
    type = MooseVariableFVReal
  []
  [reduced_pressure_error]
    type = MooseVariableFVReal
  []
  [total_pressure_error]
    type = MooseVariableFVReal
  []
[]

[AuxKernels]
  [total_pressure_audit]
    type = ParsedAux
    variable = total_pressure_audit
    coupled_variables = 'alpha pressure'
    expression = 'pressure + (alpha*${rho_l} + (1-alpha)*${rho_g})*${g}*(${domain_dims_y}-y)'
    use_xyzt = true
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [analytic_reduced_pressure]
    type = ParsedAux
    variable = analytic_reduced_pressure
    expression = 'if(x < ${dam_x} & y < ${dam_y}, -(${rho_l}-${rho_g})*${g}*(${domain_dims_y}-${dam_y}), 0)'
    use_xyzt = true
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [analytic_total_pressure]
    type = ParsedAux
    variable = analytic_total_pressure
    expression = 'if(x < ${dam_x} & y < ${dam_y}, ${rho_l}*${g}*(${dam_y}-y) + ${rho_g}*${g}*(${domain_dims_y}-${dam_y}), ${rho_g}*${g}*(${domain_dims_y}-y))'
    use_xyzt = true
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [reduced_pressure_error]
    type = ParsedAux
    variable = reduced_pressure_error
    coupled_variables = 'pressure analytic_reduced_pressure'
    expression = 'pressure - analytic_reduced_pressure'
    use_xyzt = true
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [total_pressure_error]
    type = ParsedAux
    variable = total_pressure_error
    coupled_variables = 'total_pressure_audit analytic_total_pressure'
    expression = 'total_pressure_audit - analytic_total_pressure'
    use_xyzt = true
    execute_on = 'INITIAL TIMESTEP_END'
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
  pressure_pin_point = '0.0 ${domain_dims_y} 0.0'

  startup_pressure_initialization = 'none'

  dt = 1.0e-4
  num_steps = 1
  end_time = 1.0e-4
[]

[Postprocessors]
  [cell_divergence_l2]
    type = RhieChowCellContinuityResidual
    rhie_chow_user_object = 'ins_rhie_chow_interpolator'
    metric = l2
    outputs = 'console csv'
    execute_on = 'TIMESTEP_END'
  []
  [cell_divergence_max]
    type = RhieChowCellContinuityResidual
    rhie_chow_user_object = 'ins_rhie_chow_interpolator'
    metric = max_abs
    outputs = 'console csv'
    execute_on = 'TIMESTEP_END'
  []
  [face_flux_consistency_l2]
    type = RhieChowFaceFluxConsistencyError
    rhie_chow_user_object = 'ins_rhie_chow_interpolator'
    quantity = l2
    outputs = 'console csv'
    execute_on = 'TIMESTEP_END'
  []
  [face_flux_consistency_internal]
    type = RhieChowFaceFluxConsistencyError
    rhie_chow_user_object = 'ins_rhie_chow_interpolator'
    quantity = internal_l2
    outputs = 'console csv'
    execute_on = 'TIMESTEP_END'
  []
  [correction_branch_consistency]
    type = SharpInterfaceFluxBranchConsistencyError
    rhie_chow_user_object = 'ins_rhie_chow_interpolator'
    quantity = pressure_correction
    metric = l2
    outputs = 'console csv'
    execute_on = 'TIMESTEP_END'
  []
  [total_branch_consistency]
    type = SharpInterfaceFluxBranchConsistencyError
    rhie_chow_user_object = 'ins_rhie_chow_interpolator'
    quantity = total
    metric = l2
    outputs = 'console csv'
    execute_on = 'TIMESTEP_END'
  []
[]

[VectorPostprocessors]
  [reduced_pressure_cut]
    type = LineValueSampler
    variable = pressure
    start_point = '0 ${cut_y} 0'
    end_point = '${domain_dims_x} ${cut_y} 0'
    num_points = 401
    sort_by = x
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [total_pressure_cut]
    type = LineValueSampler
    variable = total_pressure_audit
    start_point = '0 ${cut_y} 0'
    end_point = '${domain_dims_x} ${cut_y} 0'
    num_points = 401
    sort_by = x
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [analytic_reduced_pressure_cut]
    type = LineValueSampler
    variable = analytic_reduced_pressure
    start_point = '0 ${cut_y} 0'
    end_point = '${domain_dims_x} ${cut_y} 0'
    num_points = 401
    sort_by = x
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [analytic_total_pressure_cut]
    type = LineValueSampler
    variable = analytic_total_pressure
    start_point = '0 ${cut_y} 0'
    end_point = '${domain_dims_x} ${cut_y} 0'
    num_points = 401
    sort_by = x
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [reduced_pressure_error_cut]
    type = LineValueSampler
    variable = reduced_pressure_error
    start_point = '0 ${cut_y} 0'
    end_point = '${domain_dims_x} ${cut_y} 0'
    num_points = 401
    sort_by = x
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [total_pressure_error_cut]
    type = LineValueSampler
    variable = total_pressure_error
    start_point = '0 ${cut_y} 0'
    end_point = '${domain_dims_x} ${cut_y} 0'
    num_points = 401
    sort_by = x
    execute_on = 'INITIAL TIMESTEP_END'
  []
[]

[Outputs]
  csv = true
  execute_on = 'TIMESTEP_END'
[]
