mu = 1.0
rho = 1e3
mu_d = 0.3
rho_d = 1.0
dp = 0.01
U_lid = 0.0
g = -9.81
advected_interp_method = 'upwind'

T_fluid_top = 1

# Currently required
k = 1
k_d = 1
cp = 1
cp_d = 1

[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    xmin = 0
    xmax = .1
    ymin = 0
    ymax = .1
    nx = 10
    ny = 11
  []
[]

[Problem]
  linear_sys_names = 'u_system v_system pressure_system energy_system phi_system'
[]

# We are using a Materials block to order the functor creation.
# See #30827
[Materials]
  [T_from_p_h]
    type = ParsedFunctorMaterial
    property_name = 'T_from_p_h_functor'
    expression = 'h / cp_mixture'
    functor_names = 'h cp_mixture'
  []
  [h_from_p_T]
    type = ParsedFunctorMaterial
    property_name = 'h_from_p_T_functor'
    expression = 'cp_mixture * T_fluid'
    functor_names = 'T_fluid cp_mixture'
  []
  [density_1]
    type = ParsedFunctorMaterial
    property_name = 'rho'
    expression = '${rho} * (1. - 0.9 * T_fluid)'
    functor_names = 'T_fluid'
  []
  [density_2]
    type = ParsedFunctorMaterial
    property_name = 'rho_d'
    expression = '${rho_d} * (1. - 0.9 * T_fluid)'
    functor_names = 'T_fluid'
  []
[]

[Physics]
  [NavierStokes]
    [FlowSegregated]
      [flow]
        compressibility = 'weakly-compressible'

        density = 'rho_mixture'
        dynamic_viscosity = 'mu_mixture'
        gravity = '0 ${g} 0'

        # Initial conditions
        initial_velocity = '1e-12 1e-12 0'
        initial_pressure = 0.2

        wall_boundaries = 'top left right bottom'
        momentum_wall_types = 'noslip noslip noslip noslip'
        momentum_wall_functors = '${U_lid} 0; 0 0; 0 0; 0 0'

        orthogonality_correction = false
        pressure_two_term_bc_expansion = true
        momentum_advection_interpolation = ${advected_interp_method}
      []
    []
    [FluidHeatTransferSegregated]
      [heat]
        system_names = 'energy_system'
        # allows non-constant cp
        solve_for_enthalpy = true
        initial_temperature = ${T_fluid_top}

        thermal_conductivity = 'k_mixture'
        specific_heat = 'cp_mixture'

        energy_wall_boundaries = 'top          bottom'
        energy_wall_types = 'fixed-temperature fixed-temperature'
        energy_wall_functors = '0              1'

        use_nonorthogonal_correction = false
        energy_two_term_bc_expansion = true
        energy_advection_interpolation = ${advected_interp_method}
      []
    []
    [TwoPhaseMixtureSegregated]
      [mixture]
        system_names = 'phi_system'
        phase_1_fraction_name = 'phase_1'
        phase_2_fraction_name = 'phase_2'

        # not fully mixed initialization
        initial_phase_fraction = 'unstable'

        add_phase_transport_equation = true
        phase_advection_interpolation = '${advected_interp_method}'
        phase_fraction_diffusivity = 1e-3

        fluid_heat_transfer_physics = heat

        # Base phase material properties
        phase_1_density_name = 'rho'
        phase_1_viscosity_name = ${mu}
        phase_1_specific_heat_name = ${cp}
        phase_1_thermal_conductivity_name = ${k}

        # Other phase material properties
        phase_2_density_name = 'rho_d'
        phase_2_viscosity_name = ${mu_d}
        phase_2_specific_heat_name = ${cp_d}
        phase_2_thermal_conductivity_name = ${k_d}
        output_all_properties = true

        # Friction model, not actually used!
        use_dispersed_phase_drag_model = true
        particle_diameter = ${dp}
        add_advection_slip_term = false
        # To match Rayleigh Bernard nonlinear test setup
        add_gravity_term_in_slip_velocity = false
      []
    []
  []
[]

[Functions]
  [unstable]
    type = ParsedFunction
    expression = 'if(y > 0.05, 1, 0)'
  []
[]

[Executioner]
  type = PIMPLE
  rhie_chow_user_object = 'ins_rhie_chow_interpolator'

  end_time = 1e8
  [TimeStepper]
    type = IterationAdaptiveDT
    optimal_iterations = 10
    iteration_window = 2
    growth_factor = 2
    cutback_factor = 0.5
    dt = 1e-3
  []

  # Systems
  momentum_systems = 'u_system v_system'
  pressure_system = 'pressure_system'
  energy_system = 'energy_system'
  active_scalar_systems = 'phi_system'
  momentum_equation_relaxation = 0.8
  active_scalar_equation_relaxation = '0.7'
  energy_equation_relaxation = '0.5'
  pressure_variable_relaxation = 0.3

  # We need to converge the problem to show conservation
  num_iterations = 200
  pressure_absolute_tolerance = 1e-10
  momentum_absolute_tolerance = 1e-10
  energy_absolute_tolerance = 1e-10
  active_scalar_absolute_tolerance = '1e-10'
  momentum_petsc_options_iname = '-pc_type -pc_hypre_type'
  momentum_petsc_options_value = 'hypre boomeramg'
  pressure_petsc_options_iname = '-pc_type -pc_hypre_type'
  pressure_petsc_options_value = 'hypre boomeramg'
  energy_petsc_options_iname = '-pc_type -pc_hypre_type'
  energy_petsc_options_value = 'hypre boomeramg'
  active_scalar_petsc_options_iname = '-pc_type -pc_factor_shift_type' # -pc_hypre_type'
  active_scalar_petsc_options_value = 'lu NONZERO'
  momentum_l_abs_tol = 1e-13
  pressure_l_abs_tol = 1e-13
  energy_l_abs_tol = 1e-13
  active_scalar_l_abs_tol = 1e-13
  momentum_l_tol = 0
  pressure_l_tol = 0
  energy_l_tol = 0
  active_scalar_l_tol = 0
  # print_fields = true
  continue_on_max_its = true

  pin_pressure = true
  pressure_pin_value = 0.0
  pressure_pin_point = '0.0 0.0 0.0'
[]

[Outputs]
  exodus = false
  [out]
    type = CSV
    execute_on = 'FINAL'
  []
[]

[AuxVariables]
  [U]
    order = CONSTANT
    family = MONOMIAL
    fv = true
  []
[]

[Postprocessors]
  [average_void]
    type = ElementAverageValue
    variable = 'phase_2'
  []
  [max_y_velocity]
    type = ElementExtremeValue
    variable = 'vel_y'
    value_type = max
  []
  [min_y_velocity]
    type = ElementExtremeValue
    variable = 'vel_y'
    value_type = min
  []
  [max_x_velocity]
    type = ElementExtremeValue
    variable = 'vel_x'
    value_type = max
  []
  [min_x_velocity]
    type = ElementExtremeValue
    variable = 'vel_x'
    value_type = min
  []
  [max_x_slip_velocity]
    type = ElementExtremeFunctorValue
    functor = 'vel_slip_x'
    value_type = max
  []
  [max_y_slip_velocity]
    type = ElementExtremeFunctorValue
    functor = 'vel_slip_y'
    value_type = max
  []
  [max_drag_coefficient_x]
    type = ElementExtremeFunctorValue
    functor = 'Darcy_coefficient_vec_out_x'
    value_type = max
  []
  [max_drag_coefficient_y]
    type = ElementExtremeFunctorValue
    functor = 'Darcy_coefficient_vec_out_y'
    value_type = max
  []
[]
