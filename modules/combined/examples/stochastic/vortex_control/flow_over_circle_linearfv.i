!include header.i

[Mesh]
  [fmg]
    type = FileMeshGenerator
    file = flow_over_circle_linearfv_out_orig.e
    use_for_exodus_restart = true
  []
[]

[Problem]
  linear_sys_names = 'u_system v_system pressure_system'
  previous_nl_solution_required = true
[]

[Functions]
  [inlet_function]
    type = ParsedFunction
    expression = '4*U*(y-ymin)*(ymax-y)/(ymax-ymin)/(ymax-ymin)'
    symbol_names = 'U ymax ymin'
    symbol_values = '${inlet_velocity} ${y_max} ${y_min}'
  []
  [gap_x]
    type = ParsedFunction
    expression = 'Q*x/(x^2+y^2)*abs(cos(pi/(20/180*pi)*atan(x/y)))'
    symbol_names = 'Q'
    symbol_values = 'Q_signal'
  []
  [gap_y]
    type = ParsedFunction
    expression = 'if(y>0,Q,-Q)*y/(x^2+y^2)*abs(cos(pi/(20/180*pi)*atan(x/y)))'
    symbol_names = 'Q'
    symbol_values = 'Q_signal'
  []
[]

[UserObjects]
  [rc]
    type = RhieChowMassFlux
    u = vel_x
    v = vel_y
    pressure = pressure
    rho = ${rho}
    p_diffusion_kernel = p_diffusion
  []
[]

[Variables]
  [vel_x]
    type = MooseLinearVariableFVReal
    solver_sys = u_system
    initial_from_file_var = vel_x
    initial_from_file_timestep = LATEST
  []
  [vel_y]
    type = MooseLinearVariableFVReal
    solver_sys = v_system
    initial_from_file_var = vel_y
    initial_from_file_timestep = LATEST
  []
  [pressure]
    type = MooseLinearVariableFVReal
    # initial_condition = 0
    solver_sys = pressure_system
    initial_from_file_var = pressure
    initial_from_file_timestep = LATEST
  []
[]

[LinearFVKernels]
  [u_time]
    type = LinearFVTimeDerivative
    variable = vel_x
    factor = ${rho}
  []
  [u_advection_stress]
    type = LinearWCNSFVMomentumFlux
    variable = vel_x
    advected_interp_method = ${advected_interp_method}
    mu = ${mu}
    u = vel_x
    v = vel_y
    momentum_component = 'x'
    rhie_chow_user_object = 'rc'
    use_nonorthogonal_correction = true
  []
  [u_pressure]
    type = LinearFVMomentumPressure
    variable = vel_x
    pressure = pressure
    momentum_component = 'x'
  []

  [v_time]
    type = LinearFVTimeDerivative
    variable = vel_y
    factor = ${rho}
  []
  [v_advection_stress]
    type = LinearWCNSFVMomentumFlux
    variable = vel_y
    advected_interp_method = ${advected_interp_method}
    mu = ${mu}
    u = vel_x
    v = vel_y
    momentum_component = 'y'
    rhie_chow_user_object = 'rc'
    use_nonorthogonal_correction = true
  []
  [v_pressure]
    type = LinearFVMomentumPressure
    variable = vel_y
    pressure = pressure
    momentum_component = 'y'
  []

  [p_diffusion]
    type = LinearFVAnisotropicDiffusion
    variable = pressure
    diffusion_tensor = Ainv
    use_nonorthogonal_correction = true
  []
  [HbyA_divergence]
    type = LinearFVDivergence
    variable = pressure
    face_flux = HbyA
    force_boundary_execution = true
  []
[]

[LinearFVBCs]
  [inlet_x]
    type = LinearFVAdvectionDiffusionFunctorDirichletBC
    variable = vel_x
    boundary = 'left_boundary'
    functor = 'inlet_function'
  []
  [inlet_y]
    type = LinearFVAdvectionDiffusionFunctorDirichletBC
    variable = vel_y
    boundary = 'left_boundary'
    functor = 0
  []
  [circle_x]
    type = LinearFVAdvectionDiffusionFunctorDirichletBC
    variable = vel_x
    boundary = 'circle'
    functor = 0
  []
  [circle_y]
    type = LinearFVAdvectionDiffusionFunctorDirichletBC
    variable = vel_y
    boundary = 'circle'
    functor = 0
  []
  [gap_x]
    type = LinearFVAdvectionDiffusionFunctorDirichletBC
    variable = vel_x
    boundary = 'top_gap bottom_gap'
    functor = 'gap_x'
  []
  [gap_y]
    type = LinearFVAdvectionDiffusionFunctorDirichletBC
    variable = vel_y
    boundary = 'top_gap bottom_gap'
    functor = 'gap_y'
  []
  [walls_x]
    type = LinearFVAdvectionDiffusionFunctorDirichletBC
    variable = vel_x
    boundary = 'top_boundary bottom_boundary'
    functor = 0
  []
  [walls_y]
    type = LinearFVAdvectionDiffusionFunctorDirichletBC
    variable = vel_y
    boundary = 'top_boundary bottom_boundary'
    functor = 0
  []
  [outlet_p]
    type = LinearFVAdvectionDiffusionFunctorDirichletBC
    boundary = 'right_boundary'
    variable = pressure
    functor = 0
  []
  [outlet_u]
    type = LinearFVAdvectionDiffusionOutflowBC
    variable = vel_x
    use_two_term_expansion = false
    boundary = 'right_boundary'
  []
  [outlet_v]
    type = LinearFVAdvectionDiffusionOutflowBC
    variable = vel_y
    use_two_term_expansion = false
    boundary = 'right_boundary'
  []
[]

[Postprocessors]
  [drag_force]
    type = IntegralDirectedSurfaceForce
    vel_x = vel_x
    vel_y = vel_y
    mu = ${mu}
    pressure = pressure
    principal_direction = '1 0 0'
    boundary = 'circle'
    outputs = none
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [drag_coeff]
    type = ParsedPostprocessor
    expression = '2*drag_force/rho/(avgvel*avgvel)/D'
    constant_names = 'rho avgvel D'
    constant_expressions = '${rho} ${fparse 2/3*inlet_velocity} ${fparse 2*circle_radius}'
    pp_names = 'drag_force'
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [lift_force]
    type = IntegralDirectedSurfaceForce
    vel_x = vel_x
    vel_y = vel_y
    mu = ${mu}
    pressure = pressure
    principal_direction = '0 1 0'
    boundary = 'circle'
    outputs = none
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [lift_coeff]
    type = ParsedPostprocessor
    expression = '2*lift_force/rho/(avgvel*avgvel)/D'
    constant_names = 'rho avgvel D'
    constant_expressions = '${rho} ${fparse 2/3*inlet_velocity} ${fparse 2*circle_radius}'
    pp_names = 'lift_force'
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [reward]
    type = LiftDragRewardPostprocessor
    lift = lift_coeff
    drag = drag_coeff
    averaging_window = 50
    coeff_1 = 0.0
    coeff_2 = 0.2
    execute_on = 'TIMESTEP_END'
  []
  # [p1]
  #   type = PointValue
  #   variable = pressure
  #   point = '0 0.07 0.0'
  #   execute_on = 'INITIAL TIMESTEP_END'
  # []
  # [p2]
  #   type = PointValue
  #   variable = pressure
  #   point = '0 -0.07 0.0'
  #   execute_on = 'INITIAL TIMESTEP_END'
  # []
  # [p3]
  #   type = PointValue
  #   variable = pressure
  #   point = '0.075 0.1 0.0'
  #   execute_on = 'INITIAL TIMESTEP_END'
  # []
  # [p4]
  #   type = PointValue
  #   variable = pressure
  #   point = '0.075 0.0 0.0'
  #   execute_on = 'INITIAL TIMESTEP_END'
  # []
  # [p5]
  #   type = PointValue
  #   variable = pressure
  #   point = '0.075 -0.1 0.0'
  #   execute_on = 'INITIAL TIMESTEP_END'
  # []
  [p1x]
    type = PointValue
    variable = vel_x
    point = '0 0.07 0.0'
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [p2x]
    type = PointValue
    variable = vel_x
    point = '0 -0.07 0.0'
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [p3x]
    type = PointValue
    variable = vel_x
    point = '0.075 0.1 0.0'
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [p4x]
    type = PointValue
    variable = vel_x
    point = '0.075 0.0 0.0'
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [p5x]
    type = PointValue
    variable = vel_x
    point = '0.075 -0.1 0.0'
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [p1y]
    type = PointValue
    variable = vel_y
    point = '0 0.07 0.0'
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [p2y]
    type = PointValue
    variable = vel_y
    point = '0 -0.07 0.0'
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [p3y]
    type = PointValue
    variable = vel_y
    point = '0.075 0.1 0.0'
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [p4y]
    type = PointValue
    variable = vel_y
    point = '0.075 0.0 0.0'
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [p5y]
    type = PointValue
    variable = vel_y
    point = '0.075 -0.1 0.0'
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [Q_signal]
    type = ConstantPostprocessor
    value = 0.0
    execute_on = TIMESTEP_BEGIN
  []
  [Q]
    type = LibtorchControlValuePostprocessor
    control_name = src_control
  []
  [log_prob_Q]
    type = LibtorchDRLLogProbabilityPostprocessor
    control_name = src_control
  []
[]

[Reporters]
  [results]
    type = AccumulateReporter
    reporters = 'p1x/value p2x/value p3x/value p4x/value p5x/value p1y/value p2y/value p3y/value p4y/value p5y/value reward/value Q/value log_prob_Q/value'
  []
[]

[Controls]
  [src_control]
    type = LibtorchDRLControl
    parameters = "Postprocessors/Q_signal/value"
    responses = 'p1x p2x p3x p4x p5x p1y p2y p3y p4y p5y'

    # keep consistent with LibtorchDRLControlTrainer
    input_timesteps = 1
    response_scaling_factors = '13.33 15.38 16.66 38.46 15.38 33.33 40 11.76 4.711 15.38'
    response_shift_factors = '2.055 2.055 1.93 -0.171 1.945 0.449 -0.525 0.029 0.17675 1.945'
    action_scaling_factors = 0.5

    # response_scaling_factors = '1.0 1.0 1.0 1.0 1.0 1.0 1.0 1.0 1.0 1.0'
    # response_shift_factors = '0.0 0.0 0.0 0.0 0.0 0.0 0.0 0.0 0.0 0.0'
    # action_scaling_factors = 1.0

    execute_on = 'TIMESTEP_BEGIN'
    smoother = 0.1
    num_stems_in_period = 50
  []
[]

[Executioner]
  type = PIMPLE
  momentum_l_abs_tol = 1e-7
  pressure_l_abs_tol = 1e-7
  momentum_l_tol = 1e-7
  pressure_l_tol = 1e-7
  rhie_chow_user_object = 'rc'
  momentum_systems = 'u_system v_system'
  pressure_system = 'pressure_system'
  momentum_equation_relaxation = 0.9
  pressure_variable_relaxation = 0.6
  num_iterations = 100
  pressure_absolute_tolerance = 5e-6
  momentum_absolute_tolerance = 5e-6
  momentum_petsc_options_iname = '-pc_type -pc_hypre_type'
  momentum_petsc_options_value = 'hypre boomeramg'
  pressure_petsc_options_iname = '-pc_type -pc_hypre_type'
  pressure_petsc_options_value = 'hypre boomeramg'
  print_fields = false
  continue_on_max_its = true
  dt = 0.0005
  num_steps = 800
[]

[Outputs]
  exodus = true
  [json]
    type = JSON
    execute_on = final
  []
  console = false
  # execute_on = FINAL
[]
