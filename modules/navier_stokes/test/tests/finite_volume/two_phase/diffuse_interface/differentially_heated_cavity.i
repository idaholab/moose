# The differentially heated square cavity of de Vahl Davis (1983) at Ra = 1e3 and Pr = 0.71,
# solved through the diffuse-interface energy path with a single fluid: both phases carry the
# same properties and the volume fraction is 1 everywhere. The cavity is closed, so the reduced
# pressure is pinned at a corner cell. The buoyancy enters only through the Boussinesq density
# rho (1 - beta (T - T_ref)) in the gravity force. The benchmark gives an average Nusselt number
# of 1.118 on the hot and cold walls.
#
# de Vahl Davis, G. Natural convection of air in a square cavity: a bench mark numerical
# solution. Int. J. Numer. Methods Fluids 3, 249-264 (1983).

L = 1
rho = 1
cp = 1
mu = 0.01
Pr = 0.71
k = ${fparse mu * cp / Pr}
g = 9.81
T_hot = 300.5
T_cold = 299.5
T_ref = 300
Ra = 1e3
# Ra = g beta (T_hot - T_cold) L^3 / (nu a), with nu = mu / rho and a = k / (rho cp)
beta = ${fparse Ra * (mu / rho) * (k / (rho * cp)) / (g * (T_hot - T_cold) * L^3)}
n = 32

[Mesh]
  [mesh]
    type = CartesianMeshGenerator
    dim = 2
    dx = '${L}'
    dy = '${L}'
    ix = '${n}'
    iy = '${n}'
  []
[]

[Problem]
  linear_sys_names = 'u_system v_system pressure_system alpha_system energy_system'
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
        buoyancy_density = 'rho_buoyancy'
        volume_fraction_functor = 'alpha'

        initial_velocity = '0 0 0'
        initial_pressure = 0

        wall_boundaries = 'left right bottom top'
        momentum_wall_types = 'noslip noslip noslip noslip'

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
        initial_scalar_variables = '1'
        system_names = 'alpha_system'

        liquid_density_name = 'rho_phase'
        gas_density_name = 'rho_phase'
        liquid_dynamic_viscosity_name = 'mu_phase'
        gas_dynamic_viscosity_name = 'mu_phase'

        passive_scalar_advection_interpolation = 'upwind'
        compression_factor = 0
        interface_normal_functor = 'flow_interface_unit_normal_face'

        n_alpha_corrections = 1
        n_limiter_iterations = 6
      []
    []
    [ConservativeDiffuseInterfaceHeatTransferSegregated]
      [heat]
        coupled_vof_physics = 'vof'
        temperature_variable = 'T'
        liquid_fluid_properties = 'fp'
        gas_fluid_properties = 'fp'
        system_names = 'energy_system'

        initial_temperature = '${T_ref}'
        liquid_thermal_expansion = '${beta}'
        gas_thermal_expansion = '${beta}'
        reference_temperature = '${T_ref}'

        energy_wall_types = 'fixed-temperature fixed-temperature heatflux heatflux'
        energy_wall_functors = '${T_hot} ${T_cold} 0 0'
      []
    []
  []
[]

[FluidProperties]
  [fp]
    type = TemperaturePressureFunctionFluidProperties
    rho = '${rho}'
    mu = '${mu}'
    k = '${k}'
    cp = '${cp}'
  []
[]

[FunctorMaterials]
  [constants]
    type = GenericFunctorMaterial
    prop_names = 'rho_phase mu_phase'
    prop_values = '${rho} ${mu}'
  []
[]

[Executioner]
  type = ReducedPressurePIMPLE
  rhie_chow_user_object = 'ins_rhie_chow_interpolator'

  momentum_systems = 'u_system v_system'
  pressure_system = 'pressure_system'
  active_scalar_systems = 'alpha_system'
  energy_system = 'energy_system'

  # The closed cavity leaves the reduced pressure defined up to a constant
  pin_pressure = true
  pressure_pin_point = '${fparse 0.5 * L / n} ${fparse 0.5 * L / n} 0'
  pressure_pin_value = 0

  momentum_equation_relaxation = 0.7
  pressure_variable_relaxation = 0.3
  active_scalar_equation_relaxation = '1.0'
  energy_equation_relaxation = 1.0

  num_iterations = 2
  num_piso_iterations = 0
  continue_on_max_its = true
  print_fields = false

  momentum_absolute_tolerance = 1e-10
  pressure_absolute_tolerance = 1e-10
  active_scalar_absolute_tolerance = '1e-10'
  energy_absolute_tolerance = 1e-10

  momentum_l_abs_tol = 1e-12
  pressure_l_abs_tol = 1e-12
  active_scalar_l_abs_tol = 1e-12
  energy_l_abs_tol = 1e-12

  momentum_l_tol = 0
  pressure_l_tol = 0
  active_scalar_l_tol = 0
  energy_l_tol = 0

  momentum_petsc_options_iname = '-pc_type'
  momentum_petsc_options_value = 'lu'
  pressure_petsc_options_iname = '-pc_type'
  pressure_petsc_options_value = 'lu'
  active_scalar_petsc_options_iname = '-pc_type'
  active_scalar_petsc_options_value = 'lu'
  energy_petsc_options_iname = '-pc_type'
  energy_petsc_options_value = 'lu'

  dt = 2
  num_steps = 150
[]

[Postprocessors]
  # Conductive heat flux leaving the fluid through each side wall, per unit depth
  [hot_wall_flux]
    type = BoundaryLinearFVFluxIntegral
    linearfvkernels = 'heat_rho_h_diffusion'
    boundary = 'left'
    execute_on = 'TIMESTEP_END'
  []
  [cold_wall_flux]
    type = BoundaryLinearFVFluxIntegral
    linearfvkernels = 'heat_rho_h_diffusion'
    boundary = 'right'
    execute_on = 'TIMESTEP_END'
  []
  # Average Nusselt number q L / (k (T_hot - T_cold)), with the wall length L
  [Nu_hot]
    type = ParsedPostprocessor
    expression = '-hot_wall_flux / (${k} * ${fparse T_hot - T_cold})'
    pp_names = 'hot_wall_flux'
    execute_on = 'TIMESTEP_END'
  []
  [Nu_cold]
    type = ParsedPostprocessor
    expression = 'cold_wall_flux / (${k} * ${fparse T_hot - T_cold})'
    pp_names = 'cold_wall_flux'
    execute_on = 'TIMESTEP_END'
  []
  # Peak velocities in units of a / L, the benchmark's scaling
  [max_u]
    type = ElementExtremeValue
    variable = 'vel_x'
    value_type = max
    execute_on = 'TIMESTEP_END'
  []
  [max_v]
    type = ElementExtremeValue
    variable = 'vel_y'
    value_type = max
    execute_on = 'TIMESTEP_END'
  []
  [max_u_scaled]
    type = ParsedPostprocessor
    expression = 'max_u * ${L} * ${rho} * ${cp} / ${k}'
    pp_names = 'max_u'
    execute_on = 'TIMESTEP_END'
  []
  [max_v_scaled]
    type = ParsedPostprocessor
    expression = 'max_v * ${L} * ${rho} * ${cp} / ${k}'
    pp_names = 'max_v'
    execute_on = 'TIMESTEP_END'
  []
[]

[Outputs]
  [csv]
    type = CSV
    execute_on = 'FINAL'
  []
[]
