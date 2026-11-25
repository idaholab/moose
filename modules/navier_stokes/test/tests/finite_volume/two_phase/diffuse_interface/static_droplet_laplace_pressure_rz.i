# Static spherical droplet Young-Laplace pressure-jump check for the conservative
# diffuse-interface VOF path in axisymmetric (RZ) coordinates.
#
# The setup is that of static_droplet_laplace_pressure.i, with the droplet centered on the axis
# r = 0 of the r-z half plane, so it is a sphere with two equal principal curvatures and
#
#   Delta p = 2 sigma / R = 8.
#
# The axis is a symmetry boundary. The cell spacing and the interface width are those of the
# planar test, four cells across epsilon. The liquid volume error is that of the tanh profile on a
# sphere, pi^2 w^2 / (4 R^2) with w = sqrt(2) epsilon, not a solver error.

rho_l = 1.0
rho_g = 1.0
mu_l = 1.0e-2
mu_g = 1.0e-2
sigma = 1.0

xc = 0.0
yc = 0.5
R = 0.25
R_inside_sample = 0.20
R_outside_sample = 0.30
pressure_jump_exact = ${fparse 2 * sigma / R}
liquid_volume_exact = ${fparse 4 / 3 * 3.141592653589793 * R * R * R}

c_alpha = 0.1
interface_width = 0.015625

[Mesh]
  coord_type = RZ
  [mesh]
    type = CartesianMeshGenerator
    dim = 2
    dx = '0.5'
    dy = '1.0'
    ix = '128'
    iy = '256'
  []
[]

[Problem]
  linear_sys_names = 'u_system v_system pressure_system alpha_system'
  previous_nl_solution_required = true
[]

[Physics]
  [NavierStokes]
    [ConservativeDiffuseInterfaceFlowSegregated]
      [flow]
        velocity_variable = 'rhou rhov'
        pressure_variable = 'pressure'

        compressibility = 'incompressible'
        density = 'rho_mixture'
        dynamic_viscosity = 'mu_mixture'
        gravity = '0 0 0'
        volume_fraction_functor = 'alpha'
        surface_tension_coefficient = 'sigma'

        initial_velocity = '0 0 0'
        initial_pressure = '0'

        wall_boundaries = 'left right bottom'
        momentum_wall_types = 'symmetry noslip noslip'

        outlet_boundaries = 'top'
        momentum_outlet_types = 'fixed-pressure'
        pressure_functors = '0'

        orthogonality_correction = false
        momentum_two_term_bc_expansion = false
        pressure_two_term_bc_expansion = false
        momentum_advection_interpolation = 'upwind'
      []
    []
    [ConservativeDiffuseInterfaceVOFSegregated]
      [vof]
        coupled_flow_physics = 'flow'
        passive_scalar_names = 'alpha'
        initial_scalar_variables = 'alpha_init'
        system_names = 'alpha_system'

        liquid_density_name = 'rho_l'
        gas_density_name = 'rho_g'
        liquid_dynamic_viscosity_name = 'mu_l'
        gas_dynamic_viscosity_name = 'mu_g'

        passive_scalar_advection_interpolation = 'upwind'
        compression_factor = '${c_alpha}'
        interface_normal_functor = 'flow_interface_unit_normal_face'

        n_alpha_corrections = 1
        n_limiter_iterations = 6
      []
    []
  []
[]

[FunctorMaterials]
  [constants]
    type = GenericFunctorMaterial
    prop_names = 'rho_l rho_g mu_l mu_g sigma'
    prop_values = '${rho_l} ${rho_g} ${mu_l} ${mu_g} ${sigma}'
  []
[]

[Functions]
  [alpha_init]
    type = ParsedFunction
    expression = '0.5 * (1 + tanh((${R} - sqrt((x-${xc})^2 + (y-${yc})^2)) / (sqrt(2) * ${interface_width})))'
  []
[]

[AuxVariables]
  [speed]
    type = MooseVariableFVReal
  []
  [inside_pressure_sample]
    type = MooseVariableFVReal
  []
  [inside_sample]
    type = MooseVariableFVReal
  []
  [outside_pressure_sample]
    type = MooseVariableFVReal
  []
  [outside_sample]
    type = MooseVariableFVReal
  []
[]

[AuxKernels]
  [speed]
    type = ParsedAux
    variable = speed
    coupled_variables = 'rhou rhov'
    expression = 'sqrt(rhou^2 + rhov^2)'
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [inside_sample]
    type = ParsedAux
    variable = inside_sample
    expression = 'if((x-${xc})^2 + (y-${yc})^2 < ${R_inside_sample}^2, 1, 0)'
    use_xyzt = true
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [inside_pressure_sample]
    type = ParsedAux
    variable = inside_pressure_sample
    coupled_variables = 'pressure'
    expression = 'pressure * if((x-${xc})^2 + (y-${yc})^2 < ${R_inside_sample}^2, 1, 0)'
    use_xyzt = true
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [outside_sample]
    type = ParsedAux
    variable = outside_sample
    expression = 'if((x-${xc})^2 + (y-${yc})^2 > ${R_outside_sample}^2, 1, 0)'
    use_xyzt = true
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [outside_pressure_sample]
    type = ParsedAux
    variable = outside_pressure_sample
    coupled_variables = 'pressure'
    expression = 'pressure * if((x-${xc})^2 + (y-${yc})^2 > ${R_outside_sample}^2, 1, 0)'
    use_xyzt = true
    execute_on = 'INITIAL TIMESTEP_END'
  []
[]

[Executioner]
  type = ReducedPressurePIMPLE
  rhie_chow_user_object = 'ins_rhie_chow_interpolator'

  momentum_systems = 'u_system v_system'
  pressure_system = 'pressure_system'
  active_scalar_systems = 'alpha_system'

  momentum_equation_relaxation = 0.7
  pressure_variable_relaxation = 1.0
  active_scalar_equation_relaxation = '1.0'

  # The residuals reach the 1e-6 tolerances in 12 outer iterations
  num_iterations = 15
  num_piso_iterations = 0
  continue_on_max_its = true
  print_fields = false

  momentum_absolute_tolerance = 1e-6
  pressure_absolute_tolerance = 1e-6
  active_scalar_absolute_tolerance = '1e-6'

  momentum_l_abs_tol = 1e-12
  pressure_l_abs_tol = 1e-12
  active_scalar_l_abs_tol = 1e-12

  momentum_l_tol = 0
  pressure_l_tol = 0
  active_scalar_l_tol = 0

  momentum_petsc_options_iname = '-pc_type'
  momentum_petsc_options_value = 'lu'
  pressure_petsc_options_iname = '-pc_type'
  pressure_petsc_options_value = 'lu'
  active_scalar_petsc_options_iname = '-pc_type'
  active_scalar_petsc_options_value = 'lu'

  startup_flux_corrections = 2

  volume_fraction_subcycles = 1
  dt = 1.0e-4
  end_time = 1.0e-4
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
  [liquid_volume]
    type = ElementIntegralVariablePostprocessor
    variable = 'alpha'
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [liquid_volume_error]
    type = ParsedPostprocessor
    pp_names = 'liquid_volume'
    expression = 'abs(liquid_volume - volume_exact) / volume_exact'
    constant_names = 'volume_exact'
    constant_expressions = '${liquid_volume_exact}'
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [max_speed]
    type = ElementExtremeValue
    variable = 'speed'
    value_type = max
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [inside_pressure_integral]
    type = ElementIntegralVariablePostprocessor
    variable = 'inside_pressure_sample'
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [inside_sample_volume]
    type = ElementIntegralVariablePostprocessor
    variable = 'inside_sample'
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [inside_pressure_mean]
    type = ParsedPostprocessor
    pp_names = 'inside_pressure_integral inside_sample_volume'
    expression = 'inside_pressure_integral / inside_sample_volume'
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [outside_pressure_integral]
    type = ElementIntegralVariablePostprocessor
    variable = 'outside_pressure_sample'
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [outside_sample_volume]
    type = ElementIntegralVariablePostprocessor
    variable = 'outside_sample'
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [outside_pressure_mean]
    type = ParsedPostprocessor
    pp_names = 'outside_pressure_integral outside_sample_volume'
    expression = 'outside_pressure_integral / outside_sample_volume'
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [pressure_jump]
    type = ParsedPostprocessor
    pp_names = 'inside_pressure_mean outside_pressure_mean'
    expression = 'inside_pressure_mean - outside_pressure_mean'
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [pressure_jump_relative_error]
    type = ParsedPostprocessor
    pp_names = 'pressure_jump'
    expression = 'abs(pressure_jump - jump_exact) / jump_exact'
    constant_names = 'jump_exact'
    constant_expressions = '${pressure_jump_exact}'
    execute_on = 'INITIAL TIMESTEP_END'
  []
[]

[Outputs]
  execute_on = 'INITIAL TIMESTEP_END'
  [csv]
    type = CSV
  []
[]
