mu = 1.0
rho = 10.0
mu_d = 0.1
rho_d = 1.0
l = 2
U = 1
dp = 0.01
inlet_phase_2 = 0.1
advected_interp_method = 'average'

# TODO remove need for those
cp = 1
k = 1
cp_d = 1
k_d = 1

[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    xmin = 0
    xmax = '${fparse l * 5}'
    ymin = '${fparse -l / 2}'
    ymax = '${fparse l / 2}'
    nx = 10
    ny = 4
  []
  uniform_refine = 0
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
        initial_velocity = '${U} 0 0'
        initial_pressure = 0

        # Boundary conditions
        inlet_boundaries = 'left'
        momentum_inlet_types = 'fixed-velocity'
        momentum_inlet_functors = '${U} 0'

        wall_boundaries = 'top bottom'
        momentum_wall_types = 'noslip noslip'

        outlet_boundaries = 'right'
        momentum_outlet_types = 'fixed-pressure'
        pressure_functors = '0'

        # Friction is done in drift flux term
        friction_types = "Darcy"
        friction_coeffs = "Darcy_coefficient_vec"
        standard_friction_formulation = true

        momentum_advection_interpolation = '${advected_interp_method}'
        orthogonality_correction = false

        # To match reference better
        pressure_two_term_bc_expansion = true
        momentum_two_term_bc_expansion = true
      []
    []
    [TwoPhaseMixtureSegregated]
      [mixture]
        system_names = 'phi_system'
        phase_1_fraction_name = 'phase_1'
        phase_2_fraction_name = 'phase_2'

        # Phase transport equation
        add_phase_transport_equation = true
        alpha_exchange = 0.1
        phase_advection_interpolation = 'upwind'

        # see flow for inlet boundaries
        phase_fraction_inlet_type = 'fixed-value'
        phase_fraction_inlet_functors = '${inlet_phase_2}'

        # Drift flux parameters
        add_drift_flux_momentum_terms = true
        density_interp_method = 'average'
        # This has to be consistent with the friction model
        slip_linear_friction_name = 'Darcy_coefficient'

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

        # Friction model
        use_dispersed_phase_drag_model = true
        particle_diameter = ${dp}
        add_advection_slip_term = false
      []
    []
  []
[]

[Executioner]
  type = SIMPLE
  rhie_chow_user_object = 'ins_rhie_chow_interpolator'

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
  active_scalar_petsc_options_iname = '-pc_type -pc_hypre_type'
  active_scalar_petsc_options_value = 'hypre boomeramg'
  momentum_l_abs_tol = 1e-13
  pressure_l_abs_tol = 1e-13
  active_scalar_l_abs_tol = 1e-13
  momentum_l_tol = 0
  pressure_l_tol = 0
  active_scalar_l_tol = 0
  # print_fields = true
  continue_on_max_its = true
[]

[Outputs]
  print_linear_residuals = true
  print_nonlinear_residuals = true
  csv = true
  [out]
    type = Exodus
    hide = 'Re dp'
  []
  execute_on = 'INITIAL TIMESTEP_END'
[]

[Postprocessors]
  [Re]
    type = ParsedPostprocessor
    expression = '${rho} * ${l} * ${U}'
  []
  [dp]
    type = PressureDrop
    pressure = 'pressure'
    upstream_boundary = 'left'
    downstream_boundary = 'right'
    boundary = 'left right'
  []
  [average_phase2]
    type = ElementAverageValue
    variable = 'phase_2'
  []
  [max_phase2]
    type = ElementExtremeValue
    variable = 'phase_2'
  []
[]
