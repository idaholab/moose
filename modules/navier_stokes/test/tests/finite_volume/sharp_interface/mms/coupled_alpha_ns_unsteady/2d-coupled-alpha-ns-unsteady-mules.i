rho_l = 2.0
rho_g = 1.0
mu_l = 2.0e-2
mu_g = 1.0e-2
drho = ${fparse rho_l - rho_g}
dmu = ${fparse mu_l - mu_g}

u0 = 0.35
u_amp = 0.2
delta0 = 0.08
eps = ${fparse 1.0 / 9.0}
x0 = 0.35
omega = ${fparse 2.0 * pi}
c_alpha = 0.0

[Mesh]
  [mesh]
    type = GeneratedMeshGenerator
    dim = 2
    xmin = 0
    xmax = 1
    ymin = 0
    ymax = 1
    nx = 8
    ny = 9
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

        inlet_boundaries = 'left'
        momentum_inlet_types = 'fixed-velocity'
        momentum_inlet_functors = 'exact_u exact_v'

        wall_boundaries = 'top bottom'
        momentum_wall_types = 'noslip noslip'

        outlet_boundaries = 'right'
        momentum_outlet_types = 'fixed-pressure-zero-gradient'
        pressure_functors = 'exact_p'

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

        volume_fraction_inlet_functors = 'exact_alpha'
        advected_interp_method = 'upwind'
        compression_factor = 'c_alpha'
        interface_normal_functor = 'exact_interface_normal_face'

        use_mules_correction = true
        alpha_apply_prev_corr = true
        n_alpha_corrections = 3
        n_limiter_iterations = 6
      []
    []
  []
[]

[Functions]
  [exact_alpha]
    type = ParsedFunction
    expression = '0.5 * (1 - tanh((${x0} + ${u0} * (t + ${u_amp} * (1 - cos(${omega} * t)) / ${omega}) - x) / ${eps}))'
  []
  [exact_u]
    type = ParsedFunction
    expression = '4 * ${u0} * (1 + ${u_amp} * sin(${omega} * t)) * y * (1 - y) + ${delta0} * cos(${omega} * t) * sin(pi * x)^2 * 2 * y * (1 - y) * (1 - 2 * y)'
  []
  [exact_v]
    type = ParsedFunction
    expression = '-${delta0} * cos(${omega} * t) * pi * sin(2 * pi * x) * y^2 * (1 - y)^2'
  []
  [exact_p]
    type = ParsedFunction
    expression = '0'
  []
  [exact_rho]
    type = ParsedFunction
    expression = '${rho_g} + ${drho} * (0.5 * (1 - tanh((${x0} + ${u0} * (t + ${u_amp} * (1 - cos(${omega} * t)) / ${omega}) - x) / ${eps})))'
  []
  [exact_interface_x]
    type = ParsedFunction
    expression = '${x0} + ${u0} * (t + ${u_amp} * (1 - cos(${omega} * t)) / ${omega})'
  []
[]

[FunctorMaterials]
  [constants]
    type = GenericFunctorMaterial
    prop_names = 'rho_l rho_g mu_l mu_g c_alpha'
    prop_values = '${rho_l} ${rho_g} ${mu_l} ${mu_g} ${c_alpha}'
  []
  [exact_interface_normal]
    type = GenericVectorFunctorMaterial
    prop_names = 'exact_interface_normal_face'
    prop_values = '1 0 0'
  []
  [bulk_speed]
    type = ParsedFunctorMaterial
    expression = '${u0} * (1 + ${u_amp} * sin(${omega} * t))'
    property_name = 'bulk_speed'
  []
  [bulk_speed_t]
    type = ParsedFunctorMaterial
    expression = '${u0} * ${u_amp} * ${omega} * cos(${omega} * t)'
    property_name = 'bulk_speed_t'
  []
  [shear_amp]
    type = ParsedFunctorMaterial
    expression = '${delta0} * cos(${omega} * t)'
    property_name = 'shear_amp'
  []
  [shear_amp_t]
    type = ParsedFunctorMaterial
    expression = '-${delta0} * ${omega} * sin(${omega} * t)'
    property_name = 'shear_amp_t'
  []
  [alpha_x_exact]
    type = ParsedFunctorMaterial
    expression = '2 * exact_alpha * (1 - exact_alpha) / ${eps}'
    functor_names = 'exact_alpha'
    property_name = 'alpha_x_exact'
  []
  [rho_exact]
    type = ParsedFunctorMaterial
    expression = '${rho_g} + ${drho} * exact_alpha'
    functor_names = 'exact_alpha'
    property_name = 'rho_exact'
  []
  [mu_exact]
    type = ParsedFunctorMaterial
    expression = '${mu_g} + ${dmu} * exact_alpha'
    functor_names = 'exact_alpha'
    property_name = 'mu_exact'
  []
  [rho_t_exact]
    type = ParsedFunctorMaterial
    expression = '-${drho} * bulk_speed * alpha_x_exact'
    functor_names = 'bulk_speed alpha_x_exact'
    property_name = 'rho_t_exact'
  []
  [rho_x_exact]
    type = ParsedFunctorMaterial
    expression = '${drho} * alpha_x_exact'
    functor_names = 'alpha_x_exact'
    property_name = 'rho_x_exact'
  []
  [mu_x_exact]
    type = ParsedFunctorMaterial
    expression = '${dmu} * alpha_x_exact'
    functor_names = 'alpha_x_exact'
    property_name = 'mu_x_exact'
  []
  [u_x_exact]
    type = ParsedFunctorMaterial
    expression = 'shear_amp * pi * sin(2 * pi * x) * 2 * y * (1 - y) * (1 - 2 * y)'
    functor_names = 'shear_amp'
    property_name = 'u_x_exact'
  []
  [u_y_exact]
    type = ParsedFunctorMaterial
    expression = '4 * bulk_speed * (1 - 2 * y) + shear_amp * sin(pi * x)^2 * (2 - 12 * y + 12 * y^2)'
    functor_names = 'bulk_speed shear_amp'
    property_name = 'u_y_exact'
  []
  [u_xx_exact]
    type = ParsedFunctorMaterial
    expression = 'shear_amp * 2 * pi^2 * cos(2 * pi * x) * 2 * y * (1 - y) * (1 - 2 * y)'
    functor_names = 'shear_amp'
    property_name = 'u_xx_exact'
  []
  [u_yy_exact]
    type = ParsedFunctorMaterial
    expression = '-8 * bulk_speed + shear_amp * sin(pi * x)^2 * (-12 + 24 * y)'
    functor_names = 'bulk_speed shear_amp'
    property_name = 'u_yy_exact'
  []
  [u_t_exact]
    type = ParsedFunctorMaterial
    expression = '4 * bulk_speed_t * y * (1 - y) + shear_amp_t * sin(pi * x)^2 * 2 * y * (1 - y) * (1 - 2 * y)'
    functor_names = 'bulk_speed_t shear_amp_t'
    property_name = 'u_t_exact'
  []
  [v_x_exact]
    type = ParsedFunctorMaterial
    expression = '-shear_amp * 2 * pi^2 * cos(2 * pi * x) * y^2 * (1 - y)^2'
    functor_names = 'shear_amp'
    property_name = 'v_x_exact'
  []
  [v_y_exact]
    type = ParsedFunctorMaterial
    expression = '-shear_amp * pi * sin(2 * pi * x) * 2 * y * (1 - y) * (1 - 2 * y)'
    functor_names = 'shear_amp'
    property_name = 'v_y_exact'
  []
  [v_xx_exact]
    type = ParsedFunctorMaterial
    expression = '4 * pi^3 * shear_amp * sin(2 * pi * x) * y^2 * (1 - y)^2'
    functor_names = 'shear_amp'
    property_name = 'v_xx_exact'
  []
  [v_yy_exact]
    type = ParsedFunctorMaterial
    expression = '-shear_amp * pi * sin(2 * pi * x) * (2 - 12 * y + 12 * y^2)'
    functor_names = 'shear_amp'
    property_name = 'v_yy_exact'
  []
  [v_t_exact]
    type = ParsedFunctorMaterial
    expression = '-shear_amp_t * pi * sin(2 * pi * x) * y^2 * (1 - y)^2'
    functor_names = 'shear_amp_t'
    property_name = 'v_t_exact'
  []
  [forcing_alpha]
    type = ParsedFunctorMaterial
    expression = '((exact_u - bulk_speed) - ${c_alpha} * exact_u * (2 * exact_alpha - 1)) * alpha_x_exact + ${c_alpha} * u_x_exact * exact_alpha * (1 - exact_alpha)'
    functor_names = 'exact_u bulk_speed exact_alpha alpha_x_exact u_x_exact'
    property_name = 'forcing_alpha'
  []
  [forcing_u]
    type = ParsedFunctorMaterial
    expression = 'rho_t_exact * exact_u + rho_exact * u_t_exact + rho_x_exact * exact_u^2 + rho_exact * (exact_u * u_x_exact + exact_v * u_y_exact) - mu_x_exact * u_x_exact - mu_exact * (u_xx_exact + u_yy_exact)'
    functor_names = 'rho_t_exact exact_u rho_exact u_t_exact rho_x_exact exact_v u_y_exact mu_x_exact u_x_exact mu_exact u_xx_exact u_yy_exact'
    property_name = 'forcing_u'
  []
  [forcing_v]
    type = ParsedFunctorMaterial
    expression = 'rho_t_exact * exact_v + rho_exact * v_t_exact + rho_x_exact * exact_u * exact_v + rho_exact * (exact_u * v_x_exact + exact_v * v_y_exact) - mu_x_exact * v_x_exact - mu_exact * (v_xx_exact + v_yy_exact)'
    functor_names = 'rho_t_exact exact_v rho_exact v_t_exact rho_x_exact exact_u v_x_exact v_y_exact mu_x_exact mu_exact v_xx_exact v_yy_exact'
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

[AuxVariables]
  [rho_mixture_var]
    type = MooseVariableFVReal
  []
[]

[AuxKernels]
  [rho_mixture_aux]
    type = FunctorAux
    variable = rho_mixture_var
    functor = rho_mixture
    execute_on = 'TIMESTEP_END'
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
  pressure_pin_point = '1 0.5 0'
  pressure_pin_value = 0.0

  volume_fraction_subcycles = 2
  dt = 2.5e-2
  end_time = 1.0e-1
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
  [L2rho]
    type = ElementL2Error
    variable = rho_mixture_var
    function = exact_rho
    outputs = 'console csv'
    execute_on = 'timestep_end'
  []
  [alpha_min]
    type = ElementExtremeValue
    variable = alpha
    value_type = min
    outputs = 'console csv'
    execute_on = 'timestep_end'
  []
  [alpha_max]
    type = ElementExtremeValue
    variable = alpha
    value_type = max
    outputs = 'console csv'
    execute_on = 'timestep_end'
  []
  [total_alpha]
    type = ElementIntegralVariablePostprocessor
    variable = alpha
    outputs = 'console csv'
    execute_on = 'timestep_end'
  []
  [exact_total_alpha]
    type = FunctionElementIntegral
    function = exact_alpha
    outputs = 'console csv'
    execute_on = 'timestep_end'
  []
  [mass_error]
    type = ParsedPostprocessor
    expression = 'abs(total_alpha - exact_total_alpha)'
    pp_names = 'total_alpha exact_total_alpha'
    outputs = 'console csv'
    execute_on = 'timestep_end'
  []
  [interface_x]
    type = SubcellInterfacialPosition
    volume_fraction = alpha
    direction = x
    extremum_type = min
    threshold = 0.5
    secondary_min = 0.45
    secondary_max = 0.55
    outputs = 'console csv'
    execute_on = 'timestep_end'
  []
  [exact_interface_x]
    type = FunctionValuePostprocessor
    function = exact_interface_x
    outputs = 'console csv'
    execute_on = 'timestep_end'
  []
  [interface_error]
    type = ParsedPostprocessor
    expression = 'abs(interface_x - exact_interface_x)'
    pp_names = 'interface_x exact_interface_x'
    outputs = 'console csv'
    execute_on = 'timestep_end'
  []
[]

[Outputs]
  csv = true
  execute_on = 'timestep_end'
[]
