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
  []
  [vel_y]
    type = MooseLinearVariableFVReal
    solver_sys = v_system
  []
  [pressure]
    type = MooseLinearVariableFVReal
    initial_condition = 0
    solver_sys = pressure_system
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
[]

[Executioner]
  type = PIMPLE
  momentum_l_abs_tol = 1e-12
  pressure_l_abs_tol = 1e-12
  momentum_l_tol = 1e-12
  pressure_l_tol = 1e-12
  rhie_chow_user_object = 'rc'
  momentum_systems = 'u_system v_system'
  pressure_system = 'pressure_system'
  momentum_equation_relaxation = 0.90
  pressure_variable_relaxation = 0.4
  num_iterations = 100
  pressure_absolute_tolerance = 1e-10
  momentum_absolute_tolerance = 1e-10
  momentum_petsc_options_iname = '-pc_type -pc_hypre_type'
  momentum_petsc_options_value = 'hypre boomeramg'
  pressure_petsc_options_iname = '-pc_type -pc_hypre_type'
  pressure_petsc_options_value = 'hypre boomeramg'
  print_fields = false
  continue_on_max_its = true
  dt = 0.01
  num_steps = 500
[]

[Outputs]
  exodus = true
  csv = true
[]
