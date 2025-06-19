mu = 1.0
rho = 10.0
mu_d = 0.1
rho_d = 1.0
l = 2
# 'average' leads to slight oscillations, upwind may be preferred
# This method is selected for consistency with the original nonlinear input
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
  previous_nl_solution_required = true
[]

[Variables]
  [vel_x]
    type = MooseLinearVariableFVReal
    family = MONOMIAL
    fv = true
    order = CONSTANT
    solver_sys = u_system
    initial_condition = 1
  []
  [vel_y]
    type = MooseLinearVariableFVReal
    family = MONOMIAL
    fv = true
    order = CONSTANT
    solver_sys = v_system
  []
  [pressure]
    type = MooseLinearVariableFVReal
    family = MONOMIAL
    fv = true
    order = CONSTANT
    solver_sys = pressure_system
  []
[]

[LinearFVKernels]
  [flow_p_diffusion]
    type = LinearFVAnisotropicDiffusion
    diffusion_tensor = Ainv
    use_nonorthogonal_correction = false
    variable = pressure
  []
  [flow_HbyA_divergence]
    type = LinearFVDivergence
    face_flux = HbyA
    force_boundary_execution = true
    variable = pressure
  []
  [flow_ins_momentum_flux_x]
    type = LinearWCNSFVMomentumFlux
    advected_interp_method = ${advected_interp_method}
    momentum_component = x
    mu = mu_mixture
    rhie_chow_user_object = ins_rhie_chow_interpolator
    u = vel_x
    use_deviatoric_terms = false
    use_nonorthogonal_correction = false
    v = vel_y
    variable = vel_x
  []
  [flow_ins_momentum_flux_y]
    type = LinearWCNSFVMomentumFlux
    advected_interp_method = ${advected_interp_method}
    momentum_component = y
    mu = mu_mixture
    rhie_chow_user_object = ins_rhie_chow_interpolator
    u = vel_x
    use_deviatoric_terms = false
    use_nonorthogonal_correction = false
    v = vel_y
    variable = vel_y
  []
  [flow_ins_momentum_pressure_x]
    type = LinearFVMomentumPressure
    momentum_component = x
    pressure = pressure
    variable = vel_x
  []
  [flow_ins_momentum_pressure_y]
    type = LinearFVMomentumPressure
    momentum_component = y
    pressure = pressure
    variable = vel_y
  []
  [flow_momentum_friction_0_x]
    type = LinearFVMomentumFriction
    Darcy_name = Darcy_coefficient_vec
    momentum_component = x
    mu = mu_mixture
    variable = vel_x
  []
  [flow_momentum_friction_0_y]
    type = LinearFVMomentumFriction
    Darcy_name = Darcy_coefficient_vec
    momentum_component = y
    mu = mu_mixture
    variable = vel_y
  []
[]

[LinearFVBCs]
  [vel_x_left]
    type = LinearFVAdvectionDiffusionFunctorDirichletBC
    boundary = left
    functor = 1
    variable = vel_x
  []
  [vel_y_left]
    type = LinearFVAdvectionDiffusionFunctorDirichletBC
    boundary = left
    functor = 0
    variable = vel_y
  []
  [pressure_extrapolation_inlet_left]
    type = LinearFVExtrapolatedPressureBC
    boundary = left
    use_two_term_expansion = true
    variable = pressure
  []
  [vel_x_right]
    type = LinearFVAdvectionDiffusionOutflowBC
    boundary = right
    use_two_term_expansion = true
    variable = vel_x
  []
  [vel_y_right]
    type = LinearFVAdvectionDiffusionOutflowBC
    boundary = right
    use_two_term_expansion = true
    variable = vel_y
  []
  [pressure_right]
    type = LinearFVAdvectionDiffusionFunctorDirichletBC
    boundary = right
    functor = 0
    variable = pressure
  []
  [vel_x_bottom]
    type = LinearFVAdvectionDiffusionFunctorDirichletBC
    boundary = bottom
    functor = 0
    variable = vel_x
  []
  [vel_y_bottom]
    type = LinearFVAdvectionDiffusionFunctorDirichletBC
    boundary = bottom
    functor = 0
    variable = vel_y
  []
  [vel_x_top]
    type = LinearFVAdvectionDiffusionFunctorDirichletBC
    boundary = top
    functor = 0
    variable = vel_x
  []
  [vel_y_top]
    type = LinearFVAdvectionDiffusionFunctorDirichletBC
    boundary = top
    functor = 0
    variable = vel_y
  []
  [pressure_extrapolation_top_bottom]
    type = LinearFVExtrapolatedPressureBC
    boundary = 'top bottom'
    use_two_term_expansion = true
    variable = pressure
  []
[]

[FunctorMaterials]
  [flow_ins_speed_material]
    type = ADVectorMagnitudeFunctorMaterial
    execute_on = ALWAYS
    outputs = none
    vector_magnitude_name = speed
    x_functor = vel_x
    y_functor = vel_y
  []
[]

[UserObjects]
  [ins_rhie_chow_interpolator]
    type = RhieChowMassFlux
    p_diffusion_kernel = flow_p_diffusion
    pressure = pressure
    rho = rho_mixture
    u = vel_x
    v = vel_y
  []
[]

[LinearFVBCs]
  [phase_2_left]
    type = LinearFVAdvectionDiffusionFunctorDirichletBC
    boundary = left
    functor = 0.1
    variable = phase_2
  []
  [phase_2_right]
    type = LinearFVAdvectionDiffusionOutflowBC
    boundary = right
    use_two_term_expansion = true
    variable = phase_2
  []
[]

[LinearFVKernels]
  [mixture_ins_phase_2_advection]
    type = LinearFVScalarAdvection
    advected_interp_method = upwind
    rhie_chow_user_object = ins_rhie_chow_interpolator
    u_slip = vel_slip_x
    v_slip = vel_slip_y
    variable = phase_2
  []
  [mixture_phase_interface_reaction]
    type = LinearFVReaction
    coeff = 0.1
    variable = phase_2
  []
  [mixture_phase_interface_source]
    type = LinearFVSource
    scaling_factor = 0.1
    source_density = phase_1
    variable = phase_2
  []
  [mixture_drift_flux_x]
    type = LinearWCNSFV2PMomentumDriftFlux
    density_interp_method = average
    fraction_dispersed = phase_2
    momentum_component = x
    rhie_chow_user_object = ins_rhie_chow_interpolator
    rho_d = ${rho_d}
    u_slip = vel_slip_x
    v_slip = vel_slip_y
    variable = vel_x
  []
  [mixture_drift_flux_y]
    type = LinearWCNSFV2PMomentumDriftFlux
    density_interp_method = average
    fraction_dispersed = phase_2
    momentum_component = y
    rhie_chow_user_object = ins_rhie_chow_interpolator
    rho_d = ${rho_d}
    u_slip = vel_slip_x
    v_slip = vel_slip_y
    variable = vel_y
  []
[]

[FunctorMaterials]
  [mixture_phase_1_fraction]
    type = ParsedFunctorMaterial
    execute_on = ALWAYS
    expression = '1 - phase_2'
    functor_names = phase_2
    output_properties = phase_1
    outputs = all
    property_name = phase_1
  []
  [mixture_mixture_material]
    type = WCNSLinearFVMixtureFunctorMaterial
    execute_on = ALWAYS
    limit_phase_fraction = true
    outputs = all
    phase_1_fraction = phase_2
    phase_1_names = '${rho_d} ${mu_d} ${cp_d} ${k_d}'
    phase_2_names = '${rho}   ${mu}   ${cp}   ${k}'
    prop_names = 'rho_mixture mu_mixture cp_mixture k_mixture'
  []
  [mixture_slip_x]
    type = WCNSFV2PSlipVelocityFunctorMaterial
    execute_on = ALWAYS
    gravity = '0 0 0'
    linear_coef_name = Darcy_coefficient
    momentum_component = x
    mu = mu_mixture
    outputs = all
    particle_diameter = 0.01
    rho = ${rho}
    rho_d = ${rho_d}
    slip_velocity_name = vel_slip_x
    u = vel_x
    v = vel_y
  []
  [mixture_slip_y]
    type = WCNSFV2PSlipVelocityFunctorMaterial
    execute_on = ALWAYS
    gravity = '0 0 0'
    linear_coef_name = Darcy_coefficient
    momentum_component = y
    mu = mu_mixture
    outputs = all
    particle_diameter = 0.01
    rho = ${rho}
    rho_d = ${rho_d}
    slip_velocity_name = vel_slip_y
    u = vel_x
    v = vel_y
  []
  [mixture_dispersed_drag]
    type = NSFVDispersePhaseDragFunctorMaterial
    drag_coef_name = Darcy_coefficient
    execute_on = ALWAYS
    mu = mu_mixture
    outputs = all
    particle_diameter = 0.01
    rho = rho_mixture
    u = vel_x
    v = vel_y
  []
[]

[Variables]
  [phase_2]
    type = MooseLinearVariableFVReal
    family = MONOMIAL
    fv = true
    order = CONSTANT
    solver_sys = phi_system
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
  csv = true
[]

[Postprocessors]
  [Re]
    type = ParsedPostprocessor
    expression = '10.0 * 2 * 1'
  []
  [average_phase2]
    type = ElementAverageValue
    variable = phase_2
  []
  [dp]
    type = PressureDrop
    boundary = 'left right'
    downstream_boundary = right
    pressure = pressure
    upstream_boundary = left
  []
  [max_phase2]
    type = ElementExtremeValue
    variable = phase_2
  []
[]
