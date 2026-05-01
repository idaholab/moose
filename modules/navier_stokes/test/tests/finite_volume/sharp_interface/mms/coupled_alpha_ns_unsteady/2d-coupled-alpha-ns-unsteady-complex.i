rho_l = 2.0
rho_g = 1.0
mu_l = 2.0e-2
mu_g = 1.0e-2

[Mesh]
  [mesh]
    type = GeneratedMeshGenerator
    dim = 2
    xmin = 0
    xmax = 1
    ymin = 0
    ymax = 1
    nx = 4
    ny = 4
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
        gravity = '0 0 0'
        volume_fraction_functor = 'alpha'
        create_geometry_functors = false
        create_curvature_producer = false
        add_capillary_hydrostatic_flux = false
        surface_tension_coefficient = '0'

        initial_velocity = 'exact_u exact_v 0'
        initial_pressure = 'exact_p'

        wall_boundaries = 'left right top bottom'
        momentum_wall_types = 'noslip noslip noslip noslip'

        include_symmetrized_viscous_stress = false
        orthogonality_correction = false
        momentum_two_term_bc_expansion = false
        pressure_two_term_bc_expansion = false
        momentum_advection_interpolation = 'average'
      []
    []
    [SharpInterfaceVOFSegregated]
      [vof]
        coupled_flow_physics = 'flow'
        volume_fraction_variable = 'alpha'
        initial_volume_fraction = 'exact_alpha'
        system_names = 'alpha_system'

        liquid_density_name = 'rho_l'
        gas_density_name = 'rho_g'
        liquid_dynamic_viscosity_name = 'mu_l'
        gas_dynamic_viscosity_name = 'mu_g'

        advected_interp_method = 'average'
        compression_factor = '0'
        interface_normal_functor = 'unused_interface_normal'

        use_mules_correction = false
      []
    []
  []
[]

[Functions]
  [exact_alpha]
    type = ParsedFunction
    expression = '(1.0/20.0)*sin(t)*sin(2*pi*x)*sin(2*pi*y) + (1.0/10.0)*sin(pi*x)*cos(t)*cos(pi*y) + 1.0/2.0'
  []
  [exact_u]
    type = ParsedFunction
    expression = '2*pi*sin(pi*x)^2*sin(pi*y)*cos(t)*cos(pi*y)'
  []
  [exact_v]
    type = ParsedFunction
    expression = '-2*pi*sin(pi*x)*sin(pi*y)^2*cos(t)*cos(pi*x)'
  []
  [exact_p]
    type = ParsedFunction
    expression = '(1 - cos(2*pi*x))*(1 - cos(2*pi*y))*((1.0/10.0)*sin(pi*x)*sin(pi*y) + 1)*cos(t)'
  []
[]

[FunctorMaterials]
  [constants]
    type = GenericFunctorMaterial
    prop_names = 'rho_l rho_g mu_l mu_g'
    prop_values = '${rho_l} ${rho_g} ${mu_l} ${mu_g}'
  []
  [unused_interface_normal]
    type = GenericVectorFunctorMaterial
    prop_names = 'unused_interface_normal'
    prop_values = '0 0 0'
  []
  [forcing_alpha]
    type = ParsedFunctorMaterial
    expression = '(1.0/5.0)*pi^2*sin(t)*sin(pi*x)^2*sin(pi*y)*sin(2*pi*y)*cos(t)*cos(2*pi*x)*cos(pi*y) - 1.0/5.0*pi^2*sin(t)*sin(pi*x)*sin(2*pi*x)*sin(pi*y)^2*cos(t)*cos(pi*x)*cos(2*pi*y) - 1.0/10.0*sin(t)*sin(pi*x)*cos(pi*y) + (1.0/5.0)*pi^2*sin(pi*x)^2*sin(pi*y)^3*cos(t)^2*cos(pi*x) + (1.0/5.0)*pi^2*sin(pi*x)^2*sin(pi*y)*cos(t)^2*cos(pi*x)*cos(pi*y)^2 + (1.0/20.0)*sin(2*pi*x)*sin(2*pi*y)*cos(t)'
    property_name = 'forcing_alpha'
  []
  [forcing_u]
    type = ParsedFunctorMaterial
    expression = '-1.0/5.0*pi^3*mu_g*sin(t)*sin(pi*x)^2*sin(2*pi*x)*sin(pi*y)^2*cos(t)*cos(2*pi*y) - 3.0/5.0*pi^3*mu_g*sin(t)*sin(pi*x)^2*sin(2*pi*x)*sin(pi*y)*sin(2*pi*y)*cos(t)*cos(pi*y) + (1.0/5.0)*pi^3*mu_g*sin(t)*sin(pi*x)^2*sin(2*pi*x)*cos(t)*cos(pi*y)^2*cos(2*pi*y) + (2.0/5.0)*pi^3*mu_g*sin(t)*sin(pi*x)*sin(pi*y)*sin(2*pi*y)*cos(t)*cos(pi*x)*cos(2*pi*x)*cos(pi*y) + (1.0/5.0)*pi^3*mu_g*sin(t)*sin(2*pi*x)*sin(pi*y)*sin(2*pi*y)*cos(t)*cos(pi*x)^2*cos(pi*y) + (1.0/5.0)*pi^3*mu_g*sin(pi*x)^3*sin(pi*y)^3*cos(t)^2 - 7.0/5.0*pi^3*mu_g*sin(pi*x)^3*sin(pi*y)*cos(t)^2*cos(pi*y)^2 + 6*pi^3*mu_g*sin(pi*x)^2*sin(pi*y)*cos(t)*cos(pi*y) + (4.0/5.0)*pi^3*mu_g*sin(pi*x)*sin(pi*y)*cos(t)^2*cos(pi*x)^2*cos(pi*y)^2 - 2*pi^3*mu_g*sin(pi*y)*cos(t)*cos(pi*x)^2*cos(pi*y) + (1.0/5.0)*pi^3*mu_l*sin(t)*sin(pi*x)^2*sin(2*pi*x)*sin(pi*y)^2*cos(t)*cos(2*pi*y) + (3.0/5.0)*pi^3*mu_l*sin(t)*sin(pi*x)^2*sin(2*pi*x)*sin(pi*y)*sin(2*pi*y)*cos(t)*cos(pi*y) - 1.0/5.0*pi^3*mu_l*sin(t)*sin(pi*x)^2*sin(2*pi*x)*cos(t)*cos(pi*y)^2*cos(2*pi*y) - 2.0/5.0*pi^3*mu_l*sin(t)*sin(pi*x)*sin(pi*y)*sin(2*pi*y)*cos(t)*cos(pi*x)*cos(2*pi*x)*cos(pi*y) - 1.0/5.0*pi^3*mu_l*sin(t)*sin(2*pi*x)*sin(pi*y)*sin(2*pi*y)*cos(t)*cos(pi*x)^2*cos(pi*y) - 1.0/5.0*pi^3*mu_l*sin(pi*x)^3*sin(pi*y)^3*cos(t)^2 + (7.0/5.0)*pi^3*mu_l*sin(pi*x)^3*sin(pi*y)*cos(t)^2*cos(pi*y)^2 + 6*pi^3*mu_l*sin(pi*x)^2*sin(pi*y)*cos(t)*cos(pi*y) - 4.0/5.0*pi^3*mu_l*sin(pi*x)*sin(pi*y)*cos(t)^2*cos(pi*x)^2*cos(pi*y)^2 - 2*pi^3*mu_l*sin(pi*y)*cos(t)*cos(pi*x)^2*cos(pi*y) + (1.0/10.0)*pi*rho_g*sin(t)^2*sin(pi*x)^2*sin(2*pi*x)*sin(pi*y)*sin(2*pi*y)*cos(pi*y) - 2.0/5.0*pi^3*rho_g*sin(t)*sin(pi*x)^4*sin(pi*y)^2*sin(2*pi*y)*cos(t)^2*cos(2*pi*x)*cos(pi*y)^2 - 1.0/5.0*pi^3*rho_g*sin(t)*sin(pi*x)^3*sin(2*pi*x)*sin(pi*y)^4*sin(2*pi*y)*cos(t)^2*cos(pi*x) + (2.0/5.0)*pi^3*rho_g*sin(t)*sin(pi*x)^3*sin(2*pi*x)*sin(pi*y)^3*cos(t)^2*cos(pi*x)*cos(pi*y)*cos(2*pi*y) - 1.0/5.0*pi^3*rho_g*sin(t)*sin(pi*x)^3*sin(2*pi*x)*sin(pi*y)^2*sin(2*pi*y)*cos(t)^2*cos(pi*x)*cos(pi*y)^2 + (1.0/5.0)*pi*rho_g*sin(t)*sin(pi*x)^3*sin(pi*y)*cos(t)*cos(pi*y)^2 - pi*rho_g*sin(t)*sin(pi*x)^2*sin(pi*y)*cos(pi*y) - 4.0/5.0*pi^3*rho_g*sin(pi*x)^4*sin(pi*y)^4*cos(t)^3*cos(pi*x)*cos(pi*y) - 4.0/5.0*pi^3*rho_g*sin(pi*x)^4*sin(pi*y)^2*cos(t)^3*cos(pi*x)*cos(pi*y)^3 + 2*pi^3*rho_g*sin(pi*x)^3*sin(pi*y)^4*cos(t)^2*cos(pi*x) + 2*pi^3*rho_g*sin(pi*x)^3*sin(pi*y)^2*cos(t)^2*cos(pi*x)*cos(pi*y)^2 - 1.0/10.0*pi*rho_l*sin(t)^2*sin(pi*x)^2*sin(2*pi*x)*sin(pi*y)*sin(2*pi*y)*cos(pi*y) + (2.0/5.0)*pi^3*rho_l*sin(t)*sin(pi*x)^4*sin(pi*y)^2*sin(2*pi*y)*cos(t)^2*cos(2*pi*x)*cos(pi*y)^2 + (1.0/5.0)*pi^3*rho_l*sin(t)*sin(pi*x)^3*sin(2*pi*x)*sin(pi*y)^4*sin(2*pi*y)*cos(t)^2*cos(pi*x) - 2.0/5.0*pi^3*rho_l*sin(t)*sin(pi*x)^3*sin(2*pi*x)*sin(pi*y)^3*cos(t)^2*cos(pi*x)*cos(pi*y)*cos(2*pi*y) + (1.0/5.0)*pi^3*rho_l*sin(t)*sin(pi*x)^3*sin(2*pi*x)*sin(pi*y)^2*sin(2*pi*y)*cos(t)^2*cos(pi*x)*cos(pi*y)^2 - 1.0/5.0*pi*rho_l*sin(t)*sin(pi*x)^3*sin(pi*y)*cos(t)*cos(pi*y)^2 - pi*rho_l*sin(t)*sin(pi*x)^2*sin(pi*y)*cos(pi*y) + (4.0/5.0)*pi^3*rho_l*sin(pi*x)^4*sin(pi*y)^4*cos(t)^3*cos(pi*x)*cos(pi*y) + (4.0/5.0)*pi^3*rho_l*sin(pi*x)^4*sin(pi*y)^2*cos(t)^3*cos(pi*x)*cos(pi*y)^3 + 2*pi^3*rho_l*sin(pi*x)^3*sin(pi*y)^4*cos(t)^2*cos(pi*x) + 2*pi^3*rho_l*sin(pi*x)^3*sin(pi*y)^2*cos(t)^2*cos(pi*x)*cos(pi*y)^2'
    functor_names = 'rho_l rho_g mu_l mu_g'
    property_name = 'forcing_u'
  []
  [forcing_v]
    type = ParsedFunctorMaterial
    expression = '(1.0/5.0)*pi^3*mu_g*sin(t)*sin(pi*x)^2*sin(pi*y)^2*sin(2*pi*y)*cos(t)*cos(2*pi*x) + (3.0/5.0)*pi^3*mu_g*sin(t)*sin(pi*x)*sin(2*pi*x)*sin(pi*y)^2*sin(2*pi*y)*cos(t)*cos(pi*x) - 2.0/5.0*pi^3*mu_g*sin(t)*sin(pi*x)*sin(2*pi*x)*sin(pi*y)*cos(t)*cos(pi*x)*cos(pi*y)*cos(2*pi*y) - 1.0/5.0*pi^3*mu_g*sin(t)*sin(pi*x)*sin(2*pi*x)*sin(2*pi*y)*cos(t)*cos(pi*x)*cos(pi*y)^2 - 1.0/5.0*pi^3*mu_g*sin(pi*y)^2*sin(2*pi*y)*cos(t)*cos(pi*x)^2*cos(2*pi*x) + (9.0/5.0)*pi^3*mu_g*sin(pi*x)^2*sin(pi*y)^2*cos(t)^2*cos(pi*x)*cos(pi*y) - 2.0/5.0*pi^3*mu_g*sin(pi*x)^2*cos(t)^2*cos(pi*x)*cos(pi*y)^3 - 6*pi^3*mu_g*sin(pi*x)*sin(pi*y)^2*cos(t)*cos(pi*x) + 2*pi^3*mu_g*sin(pi*x)*cos(t)*cos(pi*x)*cos(pi*y)^2 - 1.0/5.0*pi^3*mu_g*sin(pi*y)^2*cos(t)^2*cos(pi*x)^3*cos(pi*y) - 1.0/5.0*pi^3*mu_l*sin(t)*sin(pi*x)^2*sin(pi*y)^2*sin(2*pi*y)*cos(t)*cos(2*pi*x) - 3.0/5.0*pi^3*mu_l*sin(t)*sin(pi*x)*sin(2*pi*x)*sin(pi*y)^2*sin(2*pi*y)*cos(t)*cos(pi*x) + (2.0/5.0)*pi^3*mu_l*sin(t)*sin(pi*x)*sin(2*pi*x)*sin(pi*y)*cos(t)*cos(pi*x)*cos(pi*y)*cos(2*pi*y) + (1.0/5.0)*pi^3*mu_l*sin(t)*sin(pi*x)*sin(2*pi*x)*sin(2*pi*y)*cos(t)*cos(pi*x)*cos(pi*y)^2 + (1.0/5.0)*pi^3*mu_l*sin(pi*y)^2*sin(2*pi*y)*cos(t)*cos(pi*x)^2*cos(2*pi*x) - 9.0/5.0*pi^3*mu_l*sin(pi*x)^2*sin(pi*y)^2*cos(t)^2*cos(pi*x)*cos(pi*y) + (2.0/5.0)*pi^3*mu_l*sin(pi*x)^2*cos(t)^2*cos(pi*x)*cos(pi*y)^3 - 6*pi^3*mu_l*sin(pi*x)*sin(pi*y)^2*cos(t)*cos(pi*x) + 2*pi^3*mu_l*sin(pi*x)*cos(t)*cos(pi*x)*cos(pi*y)^2 + (1.0/5.0)*pi^3*mu_l*sin(pi*y)^2*cos(t)^2*cos(pi*x)^3*cos(pi*y) - 1.0/10.0*pi*rho_g*sin(t)^2*sin(pi*x)*sin(2*pi*x)*sin(pi*y)^2*sin(2*pi*y)*cos(pi*x) - 1.0/5.0*pi^3*rho_g*sin(t)*sin(pi*x)^4*sin(2*pi*x)*sin(pi*y)^3*sin(2*pi*y)*cos(t)^2*cos(pi*y) + (2.0/5.0)*pi^3*rho_g*sin(t)*sin(pi*x)^3*sin(pi*y)^3*sin(2*pi*y)*cos(t)^2*cos(pi*x)*cos(2*pi*x)*cos(pi*y) - 2.0/5.0*pi^3*rho_g*sin(t)*sin(pi*x)^2*sin(2*pi*x)*sin(pi*y)^4*cos(t)^2*cos(pi*x)^2*cos(2*pi*y) - 1.0/5.0*pi^3*rho_g*sin(t)*sin(pi*x)^2*sin(2*pi*x)*sin(pi*y)^3*sin(2*pi*y)*cos(t)^2*cos(pi*x)^2*cos(pi*y) - 1.0/5.0*pi*rho_g*sin(t)*sin(pi*x)^2*sin(pi*y)^2*cos(t)*cos(pi*x)*cos(pi*y) + pi*rho_g*sin(t)*sin(pi*x)*sin(pi*y)^2*cos(pi*x) - 2.0/5.0*pi^3*rho_g*sin(pi*x)^5*sin(pi*y)^3*cos(t)^3*cos(pi*y)^2 + 2*pi^3*rho_g*sin(pi*x)^4*sin(pi*y)^3*cos(t)^2*cos(pi*y) + (2.0/5.0)*pi^3*rho_g*sin(pi*x)^3*sin(pi*y)^5*cos(t)^3*cos(pi*x)^2 + 2*pi^3*rho_g*sin(pi*x)^2*sin(pi*y)^3*cos(t)^2*cos(pi*x)^2*cos(pi*y) + (1.0/10.0)*pi*rho_l*sin(t)^2*sin(pi*x)*sin(2*pi*x)*sin(pi*y)^2*sin(2*pi*y)*cos(pi*x) + (1.0/5.0)*pi^3*rho_l*sin(t)*sin(pi*x)^4*sin(2*pi*x)*sin(pi*y)^3*sin(2*pi*y)*cos(t)^2*cos(pi*y) - 2.0/5.0*pi^3*rho_l*sin(t)*sin(pi*x)^3*sin(pi*y)^3*sin(2*pi*y)*cos(t)^2*cos(pi*x)*cos(2*pi*x)*cos(pi*y) + (2.0/5.0)*pi^3*rho_l*sin(t)*sin(pi*x)^2*sin(2*pi*x)*sin(pi*y)^4*cos(t)^2*cos(pi*x)^2*cos(2*pi*y) + (1.0/5.0)*pi^3*rho_l*sin(t)*sin(pi*x)^2*sin(2*pi*x)*sin(pi*y)^3*sin(2*pi*y)*cos(t)^2*cos(pi*x)^2*cos(pi*y) + (1.0/5.0)*pi*rho_l*sin(t)*sin(pi*x)^2*sin(pi*y)^2*cos(t)*cos(pi*x)*cos(pi*y) + pi*rho_l*sin(t)*sin(pi*x)*sin(pi*y)^2*cos(pi*x) + (2.0/5.0)*pi^3*rho_l*sin(pi*x)^5*sin(pi*y)^3*cos(t)^3*cos(pi*y)^2 + 2*pi^3*rho_l*sin(pi*x)^4*sin(pi*y)^3*cos(t)^2*cos(pi*y) - 2.0/5.0*pi^3*rho_l*sin(pi*x)^3*sin(pi*y)^5*cos(t)^3*cos(pi*x)^2 + 2*pi^3*rho_l*sin(pi*x)^2*sin(pi*y)^3*cos(t)^2*cos(pi*x)^2*cos(pi*y)'
    functor_names = 'rho_l rho_g mu_l mu_g'
    property_name = 'forcing_v'
  []
[]

[LinearFVKernels]
  [alpha_source]
    type = LinearFVSource
    variable = alpha
    source_density = forcing_alpha
  []
  [u_source]
    type = LinearFVSource
    variable = vel_x
    source_density = forcing_u
  []
  [v_source]
    type = LinearFVSource
    variable = vel_y
    source_density = forcing_v
  []
[]

[Executioner]
  type = ReducedPressurePIMPLE
  rhie_chow_user_object = 'ins_rhie_chow_interpolator'

  momentum_systems = 'u_system v_system'
  pressure_system = 'pressure_system'
  volume_fraction_systems = 'alpha_system'

  momentum_equation_relaxation = 1.0
  pressure_variable_relaxation = 1.0
  volume_fraction_equation_relaxation = '1.0'

  num_iterations = 3
  num_piso_iterations = 1
  continue_on_max_its = true
  print_fields = false

  startup_pressure_initialization = none

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
  pressure_pin_point = '0 0 0'
  pressure_pin_value = 0.0

  volume_fraction_subcycles = 1
  dt = 1e-3
  end_time = 1e-3
[]

[Postprocessors]
  [h]
    type = AverageElementSize
    outputs = 'console csv'
    execute_on = 'timestep_end'
  []
  [L2alpha]
    type = ElementL2FunctorError
    approximate = alpha
    exact = exact_alpha
    outputs = 'console csv'
    execute_on = 'timestep_end'
  []
  [L2u]
    type = ElementL2FunctorError
    approximate = vel_x
    exact = exact_u
    outputs = 'console csv'
    execute_on = 'timestep_end'
  []
  [L2v]
    type = ElementL2FunctorError
    approximate = vel_y
    exact = exact_v
    outputs = 'console csv'
    execute_on = 'timestep_end'
  []
  [L2pflux_discrete]
    type = RhieChowDiscretePressureFluxError
    rhie_chow_user_object = ins_rhie_chow_interpolator
    exact_pressure = exact_p
    outputs = 'console csv'
    execute_on = 'timestep_end'
  []
  [L2delta_u]
    type = SharpInterfacePressureCoupledVelocityError
    rhie_chow_user_object = ins_rhie_chow_interpolator
    component = x
    exact_velocity = exact_u
    outputs = 'console csv'
    execute_on = 'timestep_end'
  []
  [L2delta_v]
    type = SharpInterfacePressureCoupledVelocityError
    rhie_chow_user_object = ins_rhie_chow_interpolator
    component = y
    exact_velocity = exact_v
    outputs = 'console csv'
    execute_on = 'timestep_end'
  []
  [L2phi_consistency]
    type = RhieChowFaceFluxConsistencyError
    rhie_chow_user_object = ins_rhie_chow_interpolator
    quantity = l2
    outputs = 'console csv'
    execute_on = 'timestep_end'
  []
  [L2phi_consistency_internal]
    type = RhieChowFaceFluxConsistencyError
    rhie_chow_user_object = ins_rhie_chow_interpolator
    quantity = internal_l2
    outputs = 'console csv'
    execute_on = 'timestep_end'
  []
  [L2phi_consistency_boundary]
    type = RhieChowFaceFluxConsistencyError
    rhie_chow_user_object = ins_rhie_chow_interpolator
    quantity = boundary_l2
    outputs = 'console csv'
    execute_on = 'timestep_end'
  []
  [L2vol_phi_consistency]
    type = SharpInterfaceVolumetricFluxConsistencyError
    final_face_flux = corrected_face_phi
    vel_x = vel_x
    vel_y = vel_y
    outputs = 'console csv'
    execute_on = 'timestep_end'
  []
  [L2predictor_branch_consistency]
    type = SharpInterfaceFluxBranchConsistencyError
    rhie_chow_user_object = ins_rhie_chow_interpolator
    quantity = predictor_operator
    outputs = 'console csv'
    execute_on = 'timestep_end'
  []
  [L2correction_branch_consistency]
    type = SharpInterfaceFluxBranchConsistencyError
    rhie_chow_user_object = ins_rhie_chow_interpolator
    quantity = pressure_correction
    outputs = 'console csv'
    execute_on = 'timestep_end'
  []
  [L2total_branch_consistency]
    type = SharpInterfaceFluxBranchConsistencyError
    rhie_chow_user_object = ins_rhie_chow_interpolator
    quantity = total
    vel_x = vel_x
    vel_y = vel_y
    outputs = 'console csv'
    execute_on = 'timestep_end'
  []
[]

[Outputs]
  csv = true
  execute_on = 'timestep_end'
[]
