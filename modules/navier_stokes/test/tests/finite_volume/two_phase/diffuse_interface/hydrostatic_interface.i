# A flat, horizontal interface under gravity. The exact solution is rest, forever, so every
# velocity here is the balanced-force error of the discretisation: the capillary-free pressure
# coupling has to cancel the buoyancy source face by face. The initial reduced pressure is not the
# discrete equilibrium, so the case starts with a kick and has to decay away from it.

rho_l = 998.19
rho_g = 1.185
mu_l = 1.0e-3
mu_g = 1.48e-5
g = 9.81

a_length = 0.05715
dam_y = ${a_length}
domain_dims_x = ${fparse 10.0 * a_length}
domain_dims_y = ${fparse 1.25 * a_length}

c_alpha = 0.01

[Mesh]
  [mesh]
    type = CartesianMeshGenerator
    dim = 2
    dx = '${domain_dims_x}'
    dy = '${domain_dims_y}'
    ix = '80'
    iy = '10'
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
        velocity_variable = 'vel_x vel_y'
        pressure_variable = 'pressure'

        compressibility = 'incompressible'
        density = 'rho_mixture'
        dynamic_viscosity = 'mu_mixture'
        gravity = '0 -${g} 0'
        volume_fraction_functor = 'alpha'

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
  [alpha_interface]
    type = ParsedFunctorMaterial
    expression = 'alpha * (1 - alpha)'
    functor_names = 'alpha'
    property_name = 'alpha_interface_density'
  []
  [constants]
    type = GenericFunctorMaterial
    prop_names = 'rho_l rho_g mu_l mu_g'
    prop_values = '${rho_l} ${rho_g} ${mu_l} ${mu_g}'
  []
[]

[Functions]
  [alpha_init]
    type = ParsedFunction
    expression = 'if(y < ${dam_y}, 1, 0)'
  []
  [pressure_init]
    type = ParsedFunction
    expression = 'if(y < ${dam_y}, -(${rho_l}-${rho_g})*${g}*(${domain_dims_y}-${dam_y}), 0)'
  []
[]

[Executioner]
  type = ReducedPressurePIMPLE
  rhie_chow_user_object = 'ins_rhie_chow_interpolator'

  momentum_systems = 'u_system v_system'
  pressure_system = 'pressure_system'
  active_scalar_systems = 'alpha_system'

  momentum_equation_relaxation = 0.7
  pressure_variable_relaxation = 0.3
  active_scalar_equation_relaxation = '1.0'

  num_iterations = 1
  num_piso_iterations = 0
  continue_on_max_its = true
  print_fields = false

  momentum_absolute_tolerance = 1e-10
  pressure_absolute_tolerance = 1e-10
  active_scalar_absolute_tolerance = '1e-10'

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

  volume_fraction_subcycles = 2
  dt = 1.0e-4
  num_steps = 20
[]

[Postprocessors]
  [interface_measure]
    type = ElementIntegralFunctorPostprocessor
    functor = 'alpha_interface_density'
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [max_vx]
    type = ElementExtremeValue
    variable = 'vel_x'
    value_type = max
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [max_vy]
    type = ElementExtremeValue
    variable = 'vel_y'
    value_type = max
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [min_vy]
    type = ElementExtremeValue
    variable = 'vel_y'
    value_type = min
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
  [total_alpha]
    type = ElementIntegralVariablePostprocessor
    variable = 'alpha'
    execute_on = 'INITIAL TIMESTEP_END'
  []
[]

[Outputs]
  execute_on = 'INITIAL TIMESTEP_END'
  [csv]
    type = CSV
  []
[]
