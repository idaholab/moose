mu = 1.0
rho = 1.0e3
mu_d = 0.3
rho_d = 5e2
dp = 0.01
U_lid = 0.1
advected_interp_method = 'upwind'

k = 1
k_d = 1
cp = 1
cp_d = 1

[Mesh]
  inactive = 'make_skew'
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    xmin = 0
    xmax = .1
    ymin = 0
    ymax = .1
    nx = 10
    ny = 10
  []
  [make_skew]
    type = MoveNodeGenerator
    input = 'gen'
    node_id = 0
    shift_position = '0.007 0.0021 0'
  []
[]

[Problem]
  linear_sys_names = 'u_system v_system pressure_system phi_system'
[]

[Physics]
  [NavierStokes]
    [FlowSegregated]
      [flow]
        compressibility = 'incompressible'

        density = 'rho_mixture'
        dynamic_viscosity = 'mu_mixture'

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
    [TwoPhaseMixtureSegregated]
      [mixture]
        system_names = 'phi_system'
        phase_1_fraction_name = 'phase_1'
        phase_2_fraction_name = 'phase_2'

        add_phase_transport_equation = true
        phase_advection_interpolation = '${advected_interp_method}'
        phase_fraction_diffusivity = 1e-3

        # We could consider adding fixed-value-yet-not-an-inlet
        # boundary conditions to the TwoPhaseMixture physics

        # Base phase material properties
        phase_1_density_name = ${rho}
        phase_1_viscosity_name = ${mu}
        phase_1_specific_heat_name = ${cp}
        phase_1_thermal_conductivity_name = ${k}

        # Other phase material properties
        phase_2_density_name = ${rho_d}
        phase_2_viscosity_name = ${mu_d}
        phase_2_specific_heat_name = ${cp_d}
        phase_2_thermal_conductivity_name = ${k_d}
        output_all_properties = true

        # Friction model, not actually used!
        use_dispersed_phase_drag_model = true
        particle_diameter = ${dp}
        add_advection_slip_term = false
      []
    []
  []
[]

[LinearFVBCs]
  [botttom-phase-2]
    type = LinearFVAdvectionDiffusionFunctorDirichletBC
    boundary = 'bottom'
    variable = phase_2
    functor = '0'
  []
  [top-phase-2]
    type = LinearFVAdvectionDiffusionFunctorDirichletBC
    boundary = 'top'
    variable = phase_2
    functor = '1'
  []
[]

[Executioner]
  type = PIMPLE
  rhie_chow_user_object = 'ins_rhie_chow_interpolator'
  dt = 1
  end_time = 10

  # Systems
  momentum_systems = 'u_system v_system'
  pressure_system = 'pressure_system'
  active_scalar_systems = 'phi_system'
  momentum_equation_relaxation = 0.8
  active_scalar_equation_relaxation = '0.7'
  pressure_variable_relaxation = 0.3

  # We need to converge the problem to show conservation
  num_iterations = 200
  pressure_absolute_tolerance = 1e-10
  momentum_absolute_tolerance = 1e-10
  active_scalar_absolute_tolerance = '1e-10'
  momentum_petsc_options_iname = '-pc_type -pc_hypre_type'
  momentum_petsc_options_value = 'hypre boomeramg'
  pressure_petsc_options_iname = '-pc_type -pc_hypre_type'
  pressure_petsc_options_value = 'hypre boomeramg'
  active_scalar_petsc_options_iname = '-pc_type -pc_factor_shift_type' # -pc_hypre_type'
  active_scalar_petsc_options_value = 'lu NONZERO'
  momentum_l_abs_tol = 1e-13
  pressure_l_abs_tol = 1e-13
  active_scalar_l_abs_tol = 1e-13
  momentum_l_tol = 0
  pressure_l_tol = 0
  active_scalar_l_tol = 0
  # print_fields = true
  continue_on_max_its = true

  pin_pressure = true
  pressure_pin_value = 0.0
  pressure_pin_point = '0.05 0.05 0.0'
[]

[Outputs]
  exodus = true
  [out]
    type = CSV
    execute_on = 'FINAL'
  []
[]

[AuxVariables]
  [drag_coefficient]
    type = MooseVariableFVReal
  []
[]

[AuxKernels]
  [populate_cd]
    type = FunctorAux
    variable = drag_coefficient
    functor = 'Darcy_coefficient'
    execute_on = 'TIMESTEP_END'
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
  [max_drag_coefficient]
    type = ElementExtremeFunctorValue
    functor = 'drag_coefficient'
    value_type = max
  []
[]
