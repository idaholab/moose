mu = 1.1
rho = 1.2
advected_interp_method = 'average'

[Problem]
  linear_sys_names = 'u_system v_system pressure_system'
  previous_nl_solution_required = true
[]


[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 2
    ny = 2
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
    initial_condition = 0.0
    solver_sys = u_system
  []
  [vel_y]
    type = MooseLinearVariableFVReal
    solver_sys = v_system
    initial_condition = 0.0
  []
  [pressure]
    type = MooseLinearVariableFVReal
    solver_sys = pressure_system
    initial_condition = 0
  []
[]

[LinearFVKernels]
  [u_advection_stress]
    type = LinearWCNSFVMomentumFlux
    variable = vel_x
    advected_interp_method = ${advected_interp_method}
    mu = ${mu}
    u = vel_x
    v = vel_y
    momentum_component = 'x'
    rhie_chow_user_object = 'rc'
    use_nonorthogonal_correction = false
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
    use_nonorthogonal_correction = false
  []
  [u_pressure]
    type = LinearFVMomentumPressure
    variable = vel_x
    pressure = pressure
    momentum_component = 'x'
  []
  [v_pressure]
    type = LinearFVMomentumPressure
    variable = vel_y
    pressure = pressure
    momentum_component = 'y'
  []
  [u_forcing]
    type = LinearFVSource
    variable = vel_x
    source_density = forcing_u
  []
  [v_forcing]
    type = LinearFVSource
    variable = vel_y
    source_density = forcing_v
  []
  [p_diffusion]
    type = LinearFVAnisotropicDiffusion
    variable = pressure
    diffusion_tensor = Ainv
    use_nonorthogonal_correction = false
    use_nonorthogonal_correction_on_boundary = false
  []
  [HbyA_divergence]
    type = LinearFVDivergence
    variable = pressure
    face_flux = HbyA
    force_boundary_execution = true
  []
[]

[LinearFVBCs]
  [no-slip-wall-u]
    type = LinearFVAdvectionDiffusionFunctorDirichletBC
    boundary = 'left right top'
    variable = vel_x
    functor = exact_u
  []
  [no-slip-wall-v]
    type = LinearFVAdvectionDiffusionFunctorDirichletBC
    boundary = 'left right top'
    variable = vel_y
    functor = exact_v
  []
  [symmetry-u]
    type = LinearFVVelocitySymmetryBC
    boundary = 'bottom'
    variable = vel_x
    u = vel_x
    v = vel_y
    momentum_component = x
  []
  [symmetry-v]
    type = LinearFVVelocitySymmetryBC
    boundary = 'bottom'
    variable = vel_y
    u = vel_x
    v = vel_y
    momentum_component = y
  []
  [pressure-extrapolation]
    type = LinearFVExtrapolatedPressureBC
    boundary = 'left right top'
    variable = pressure
    use_two_term_expansion = true
  []
  [pressure-symmetry]
    type = LinearFVScalarSymmetryBC
    boundary = 'bottom'
    variable = pressure
    use_two_term_expansion = false
  []
[]

[Functions]
  [exact_u]
    type = ParsedFunction
    expression = '(x-x^2)*(1-2*y^2)'
  []
  [exact_v]
    type = ParsedFunction
    expression = '-(y-2/3*y^3)*(1-2*x)'
  []
  [exact_p]
    type = ParsedFunction
    expression = 'y^2'
  []
  [forcing_u]
    type = ParsedFunction
    expression = '2*mu*(1-2*y^2)+rho*(x-x^2)*(1-2*y^2)*(1-2*x)*(1-2*y^2)+rho*(-y+2/3*y^3)*(1-2*x)*(x-x^2)*(-4*y)'
    symbol_names = 'mu rho'
    symbol_values = '${mu} ${rho}'
  []
  [forcing_v]
    type = ParsedFunction
    expression = '-mu*4*y*(1-2*x)+rho*(x-x^2)*(1-2*y^2)*(-y+2/3*y^3)*(-2)+rho*(-y+2/3*y^3)*(-1+2*y^2)*(1-2*x)+2*y'
    symbol_names = 'mu rho'
    symbol_values = '${mu} ${rho}'
  []
[]

[Executioner]
  type = SIMPLE
  momentum_l_abs_tol = 1e-8
  pressure_l_abs_tol = 1e-8
  momentum_l_tol = 0
  pressure_l_tol = 0
  rhie_chow_user_object = 'rc'
  momentum_systems = 'u_system v_system'
  pressure_system = 'pressure_system'
  momentum_equation_relaxation = 0.8
  pressure_variable_relaxation = 0.3
  num_iterations = 2000
  pressure_absolute_tolerance = 1e-8
  momentum_absolute_tolerance = 1e-8
  momentum_petsc_options_iname = '-pc_type -pc_hypre_type'
  momentum_petsc_options_value = 'hypre boomeramg'
  pressure_petsc_options_iname = '-pc_type -pc_hypre_type'
  pressure_petsc_options_value = 'hypre boomeramg'
  print_fields = false

  pin_pressure = true
  pressure_pin_value = 0.25
  pressure_pin_point = '0.5 0.5 0.0'
[]

[Outputs]
  exodus = true
  [csv]
    type = CSV
    execute_on = FINAL
  []
[]

[Postprocessors]
  [h]
    type = AverageElementSize
    outputs = 'csv'
    execute_on = FINAL
  []
  [L2u]
    type = ElementL2FunctorError
    approximate = vel_x
    exact = exact_u
    outputs = 'csv'
    execute_on = FINAL
  []
  [L2v]
    type = ElementL2FunctorError
    approximate = vel_y
    exact = exact_v
    outputs = 'csv'
    execute_on = FINAL
  []
  [L2p]
    approximate = pressure
    exact = exact_p
    type = ElementL2FunctorError
    outputs = 'csv'
    execute_on = FINAL
  []
[]
