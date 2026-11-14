# Steady conduction through a liquid layer under a gas layer at rest, heated from below and cooled
# from above. The sides are adiabatic, so the sharp-interface temperature is linear in each layer
# and carries the same heat flux q = (T_hot - T_cold) / (H_l / k_l + H_g / k_g) through both.
# The volume fraction ramps linearly across four cells centered on the interface, holding the
# liquid volume H_l. The phases conduct in series within a cell and the face conductivity is the
# harmonic mean, so the cell resistances add up to the two layers' exactly: the temperature of
# every pure cell outside the ramp is the sharp-interface one to round-off. Volume-weighted cell
# conductivity or arithmetic face interpolation each leave errors of order 1 K there. Without
# gravity and with equal phase densities the fluid stays at rest.

rho = 1
mu = 1
H = 1
H_l = 0.5
T_hot = 400
T_cold = 300
# A conductivity ratio of 10 makes the interface face carry most of the resistance
k_l = 1
k_g = 0.1
# Four of the 20 cells across the height
interface_width = 0.2
q = ${fparse (T_hot - T_cold) / (H_l / k_l + (H - H_l) / k_g)}
T_interface = ${fparse T_hot - q * H_l / k_l}

[Mesh]
  # Cells twice as wide as tall, so the horizontal and vertical faces have different areas
  [mesh]
    type = CartesianMeshGenerator
    dim = 2
    dx = '0.2'
    dy = '${H}'
    ix = '2'
    iy = '20'
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
        gravity = '0 0 0'
        volume_fraction_functor = 'alpha'

        initial_velocity = '0 0 0'
        initial_pressure = 0

        wall_boundaries = 'bottom top left'
        momentum_wall_types = 'noslip noslip noslip'

        outlet_boundaries = 'right'
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
        compression_factor = 0
        interface_normal_functor = 'flow_interface_unit_normal_face'

        n_alpha_corrections = 1
        n_limiter_iterations = 6
      []
    []
    [ConservativeDiffuseInterfaceHeatTransferSegregated]
      [heat]
        coupled_vof_physics = 'vof'
        conserved_enthalpy_variable = 'rho_h'
        temperature_variable = 'T'
        liquid_fluid_properties = 'fp_l'
        gas_fluid_properties = 'fp_g'
        system_names = 'energy_system'

        initial_temperature = '${T_cold}'

        energy_wall_types = 'fixed-temperature fixed-temperature heatflux'
        energy_wall_functors = '${T_hot} ${T_cold} 0'
      []
    []
  []
[]

[FluidProperties]
  # The steady temperature does not depend on cp; the linear cp(T) keeps the T recovery nonlinear
  # on the way there
  [fp_l]
    type = TemperaturePressureFunctionFluidProperties
    rho = '${rho}'
    mu = '${mu}'
    k = '${k_l}'
    cp = cp_l
  []
  [fp_g]
    type = TemperaturePressureFunctionFluidProperties
    rho = '${rho}'
    mu = '${mu}'
    k = '${k_g}'
    cp = cp_g
  []
[]

[FunctorMaterials]
  [constants]
    type = GenericFunctorMaterial
    prop_names = 'rho_l rho_g mu_l mu_g'
    prop_values = '${rho} ${rho} ${mu} ${mu}'
  []
  # The temperature error in the pure cells, outside any diffuse interface band
  [pure_cell_error]
    type = ParsedFunctorMaterial
    expression = 'if(alpha * (1 - alpha) > 0, 0, abs(T - T_exact))'
    functor_names = 'alpha T T_exact'
    property_name = 'pure_cell_T_error'
  []
[]

[Functions]
  # Symmetric about y = H_l, so the cells hold exactly the liquid volume H_l
  [alpha_init]
    type = ParsedFunction
    expression = 'max(0, min(1, 0.5 - (y - ${H_l}) / ${interface_width}))'
  []
  # TemperaturePressureFunctionFluidProperties evaluates its functions at (x, y) = (T, p)
  [cp_l]
    type = ParsedFunction
    expression = '1 + 0.01 * x'
  []
  [cp_g]
    type = ParsedFunction
    expression = '0.5 + 0.002 * x'
  []
  [T_exact]
    type = ParsedFunction
    expression = 'if(y < ${H_l}, ${T_hot} - ${q} * y / ${k_l},
                                 ${T_interface} - ${q} * (y - ${H_l}) / ${k_g})'
  []
[]

[Executioner]
  type = ReducedPressurePIMPLE
  rhie_chow_user_object = 'ins_rhie_chow_interpolator'

  momentum_systems = 'u_system v_system'
  pressure_system = 'pressure_system'
  active_scalar_systems = 'alpha_system'
  energy_system = 'energy_system'

  momentum_equation_relaxation = 0.7
  pressure_variable_relaxation = 0.3
  active_scalar_equation_relaxation = '1.0'
  energy_equation_relaxation = 1.0

  num_iterations = 1
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

  # About 300 times the diffusion time of the slower gas layer, H_g^2 rho cp / k ~ 3, so each
  # implicit step reduces the transient by about 1e-3 and the steady state is reached to
  # round-off in five steps
  dt = 1e3
  num_steps = 6
[]

[Postprocessors]
  [T_error]
    type = ElementL2FunctorError
    approximate = 'T'
    exact = 'T_exact'
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [max_pure_cell_T_error]
    type = ElementExtremeFunctorValue
    functor = 'pure_cell_T_error'
    value_type = max
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [T_min]
    type = ElementExtremeValue
    variable = 'T'
    value_type = min
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [T_max]
    type = ElementExtremeValue
    variable = 'T'
    value_type = max
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [max_speed]
    type = ElementExtremeFunctorValue
    functor = 'vel_y'
    value_type = max
    execute_on = 'INITIAL TIMESTEP_END'
  []
[]

[Outputs]
  execute_on = 'INITIAL TIMESTEP_END'
  [csv]
    type = CSV
  []
[]
