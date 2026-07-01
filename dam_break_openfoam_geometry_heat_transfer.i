# OpenFOAM-parity sharp-interface VOF dam-break case.
# Geometry matches Rod's OpenFOAM damBreak_MM_a2250 setup:
#   a = 0.05715 m
#   domain = 10a x 1.25a
#   mesh = 400 x 50
#   initial water box = [0, a] x [0, a]

rho_l = 998.19
rho_g = 1.185
mu_l = 1.0e-3
mu_g = 1.48e-5
cp_l = 4182
cp_g = 1005
k_l = 0.6
k_g = 0.026
g = 9.81

a_length = 0.05715
dam_x = ${a_length}
dam_y = ${a_length}
domain_dims_x = ${fparse 10.0 * a_length}
domain_dims_y = ${fparse 1.25 * a_length}

c_alpha = 0.01
tracer_c0 = 1.0
tracer_patch_x_min = ${fparse 0.75 * dam_x}
tracer_patch_x_max = ${dam_x}
tracer_patch_y = ${fparse 0.75 * dam_y}
T_hot_liquid = 350
T_air = 300
T_bottom = 50

[Mesh]
  [mesh]
    type = CartesianMeshGenerator
    dim = 2
    dx = '${domain_dims_x}'
    dy = '${domain_dims_y}'
    ix = '400'
    iy = '50'
  []
[]

[Problem]
  linear_sys_names = 'u_system v_system pressure_system alpha_system tracer_system energy_system'
  previous_nl_solution_required = true
[]

[Variables]
  [tracer_amount]
    type = MooseLinearVariableFVReal
    solver_sys = tracer_system
  []
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
        passive_scalar_names = 'alpha'
        initial_scalar_variables = 'alpha_init'
        system_names = 'alpha_system'
        volume_fraction_outlet_type = 'inlet-outlet'

        liquid_density_name = 'rho_l'
        gas_density_name = 'rho_g'
        liquid_specific_heat_name = 'cp_l'
        gas_specific_heat_name = 'cp_g'
        rho_cp_phi_name = 'rho_cp_phi'
        liquid_dynamic_viscosity_name = 'mu_l'
        gas_dynamic_viscosity_name = 'mu_g'

        passive_scalar_advection_interpolation = 'upwind'
        compression_factor = '${c_alpha}'
        interface_normal_functor = 'flow_interface_unit_normal_face'
        confined_scalar_variables = 'tracer_amount'
        confined_scalar_concentration_min = 0
        confined_scalar_concentration_max = 1
        thermal_energy_variable = 'thermal_energy'
        thermal_energy_temperature = 'temperature'
        thermal_energy_backflow_temperature = 'T_air'

        n_alpha_corrections = 1
        n_limiter_iterations = 6
      []
    []
    [FluidHeatTransferSegregated]
      [heat]
        coupled_flow_physics = 'flow'
        system_names = 'energy_system'
        fluid_temperature_variable = 'temperature'
        solve_for_conserved_energy = true
        fluid_conserved_energy_variable = 'thermal_energy'

        thermal_conductivity = 'k_mixture'
        specific_heat = 'cp_mixture'
        energy_mass_heat_capacity_face_flux = 'rho_cp_phi'
        energy_mass_heat_capacity_face_flux_is_integrated = true
        use_vof_consistent_energy_advection = true

        initial_temperature = 'temperature_init'
        initial_conserved_energy = 'thermal_energy_init'

        energy_wall_boundaries = 'bottom'
        energy_wall_types = 'fixed-temperature'
        energy_wall_functors = 'T_bottom'

        use_nonorthogonal_correction = false
        energy_two_term_bc_expansion = false
        energy_advection_interpolation = 'upwind'
      []
    []
  []
[]

[FunctorMaterials]
  [constants]
    type = GenericFunctorMaterial
    prop_names = 'rho_l rho_g mu_l mu_g cp_l cp_g k_l k_g T_air T_bottom'
    prop_values = '${rho_l} ${rho_g} ${mu_l} ${mu_g} ${cp_l} ${cp_g} ${k_l} ${k_g} ${T_air} ${T_bottom}'
  []
  [k_mixture]
    type = ParsedFunctorMaterial
    property_name = 'k_mixture'
    expression = 'min(max(alpha, 0), 1) * k_l + (1 - min(max(alpha, 0), 1)) * k_g'
    functor_names = 'alpha k_l k_g'
  []
  [cp_mixture]
    type = ParsedFunctorMaterial
    property_name = 'cp_mixture'
    expression = '(min(max(alpha, 0), 1) * rho_l * cp_l + (1 - min(max(alpha, 0), 1)) * rho_g * cp_g) / (min(max(alpha, 0), 1) * rho_l + (1 - min(max(alpha, 0), 1)) * rho_g)'
    functor_names = 'alpha rho_l rho_g cp_l cp_g'
  []
  [rho_cp]
    type = ParsedFunctorMaterial
    property_name = 'rho_cp'
    expression = 'rho_mixture * cp_mixture'
    functor_names = 'rho_mixture cp_mixture'
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
  [tracer_amount_init]
    type = ParsedFunction
    expression = 'if(x > ${tracer_patch_x_min} & x < ${tracer_patch_x_max} & y > ${tracer_patch_y} & y < ${dam_y}, ${tracer_c0}, 0)'
  []
  [temperature_init]
    type = ParsedFunction
    expression = 'if(x < ${dam_x} & y < ${dam_y}, ${T_hot_liquid}, ${T_air})'
  []
  [thermal_energy_init]
    type = ParsedFunction
    expression = 'if(x < ${dam_x} & y < ${dam_y}, ${rho_l} * ${cp_l} * ${T_hot_liquid}, ${rho_g} * ${cp_g} * ${T_air})'
  []
[]

[ICs]
  [tracer_amount]
    type = FunctionIC
    variable = tracer_amount
    function = tracer_amount_init
  []
[]

[Executioner]
  type = ReducedPressurePIMPLE
  rhie_chow_user_object = 'ins_rhie_chow_interpolator'

  momentum_systems = 'u_system v_system'
  pressure_system = 'pressure_system'
  energy_system = 'energy_system'
  active_scalar_systems = 'alpha_system'

  momentum_equation_relaxation = 0.7
  energy_equation_relaxation = 0.7
  pressure_variable_relaxation = 0.3
  active_scalar_equation_relaxation = '1.0'

  num_iterations = 1
  num_piso_iterations = 0
  continue_on_max_its = true
  print_fields = false

  momentum_absolute_tolerance = 1e-10
  pressure_absolute_tolerance = 1e-10
  energy_absolute_tolerance = 1e-10
  active_scalar_absolute_tolerance = '1e-10'

  momentum_l_abs_tol = 1e-12
  pressure_l_abs_tol = 1e-12
  energy_l_abs_tol = 1e-12
  active_scalar_l_abs_tol = 1e-12

  momentum_l_tol = 0
  pressure_l_tol = 0
  energy_l_tol = 0
  active_scalar_l_tol = 0

  momentum_petsc_options_iname = '-pc_type'
  momentum_petsc_options_value = 'lu'
  pressure_petsc_options_iname = '-pc_type'
  pressure_petsc_options_value = 'lu'
  energy_petsc_options_iname = '-pc_type'
  energy_petsc_options_value = 'lu'
  active_scalar_petsc_options_iname = '-pc_type'
  active_scalar_petsc_options_value = 'lu'

  startup_pressure_initialization = 'projection-only'
  startup_flux_corrections = 2

  volume_fraction_subcycles = 2
  dt = 1.0e-4
  end_time = 0.2
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
  [total_alpha]
    type = ElementIntegralVariablePostprocessor
    variable = 'alpha'
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [total_tracer_amount]
    type = ElementIntegralVariablePostprocessor
    variable = 'tracer_amount'
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [min_temperature]
    type = ElementExtremeValue
    variable = 'temperature'
    value_type = min
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [max_temperature]
    type = ElementExtremeValue
    variable = 'temperature'
    value_type = max
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [total_heat]
    type = ElementIntegralVariablePostprocessor
    variable = 'thermal_energy'
    execute_on = 'INITIAL TIMESTEP_END'
  []
[]

[Outputs]
  execute_on = 'INITIAL TIMESTEP_END'
  file_base = dam_break_openfoam_geometry_heat_transfer
  [csv]
    type = CSV
    time_step_interval = 1
  []
  [exodus]
    type = Exodus
    time_step_interval = 1
  []
[]
