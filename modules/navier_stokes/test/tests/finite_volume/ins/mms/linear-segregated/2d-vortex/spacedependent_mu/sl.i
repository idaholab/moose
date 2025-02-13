rho = 1
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
    mu = 'mu'
    u = vel_x
    v = vel_y
    momentum_component = 'x'
    rhie_chow_user_object = 'rc'
    use_nonorthogonal_correction = false
    use_deviatoric_terms = false
  []
  [v_advection_stress]
    type = LinearWCNSFVMomentumFlux
    variable = vel_y
    advected_interp_method = ${advected_interp_method}
    mu = 'mu'
    u = vel_x
    v = vel_y
    momentum_component = 'y'
    rhie_chow_user_object = 'rc'
    use_nonorthogonal_correction = false
    use_deviatoric_terms = false
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
    boundary = 'left right top bottom'
    variable = vel_x
    functor = '0'
  []
  [no-slip-wall-v]
    type = LinearFVAdvectionDiffusionFunctorDirichletBC
    boundary = 'left right top bottom'
    variable = vel_y
    functor = '0'
  []
  [pressure-extrapolation]
    type = LinearFVExtrapolatedPressureBC
    boundary = 'left right top bottom'
    variable = pressure
    use_two_term_expansion = true
  []
[]

[Functions]
  [exact_u]
    type = ParsedFunction
    expression = 'x^2*(1-x)^2*(2*y-6*y^2+4*y^3)'
  []
  [exact_v]
    type = ParsedFunction
    expression = '-y^2*(1-y)^2*(2*x-6*x^2+4*x^3)'
  []
  [exact_p]
    type = ParsedFunction
    expression = 'x*(1-x)'
  []
  [mu]
    type = ParsedFunction
    expression = '1+(x-1)*x*(y-1)*y'
  []
  [forcing_u]
    type = ParsedFunction
    expression = '-(2*x-1)*y*(y-1)*(2*x-6*x^2+4*x^3)*(2*y-6*y^2+4*y^3)'
                 '-(1+x*(x-1)*y*(y-1))*(2*y-6*y^2+4*y^3)*(2-12*x+12*x^2)'
                 '-(2*y-1)*x*(x-1)*(x^2*(1-x)^2*(2-12*y+12*y^2))'
                 '-(1+x*(x-1)*y*(y-1))*(x^2*(1-x)^2*(-12+24*y))'
                 '+1-2*x+rho*4*x^3*y^2*(2*y^2-2*y+1)*(y-1)^2*(-1+2*x)*(x-1)^3'
    symbol_names = 'rho'
    symbol_values = '${rho}'
  []
  [forcing_v]
    type = ParsedFunction
    expression = '(2*y-1)*x*(x-1)*(2*y-6*y^2+4*y^3)*(2*x-6*x^2+4*x^3)'
                 '+(1+x*(x-1)*y*(y-1))*(2-12*y+12*y^2)*(2*x-6*x^2+4*x^3)'
                 '+(2*x-1)*y*(y-1)*(y^2*(1-y)^2*(2-12*x+12*x^2))'
                 '+(1+x*(x-1)*y*(y-1))*(y^2*(1-y)^2*(-12+24*x))'
                 '+rho*4*y^3*x^2*(2*x^2-2*x+1)*(x-1)^2*(-1+2*y)*(y-1)^3'
    symbol_names = 'rho'
    symbol_values = '${rho}'
  []
  [forcing_u_deviatoric]
    type = ParsedFunction
    expression = '-2*(2*x-1)*y*(y-1)*(2*x-6*x^2+4*x^3)*(2*y-6*y^2+4*y^3)'
                 '-2*(1+x*(x-1)*y*(y-1))*(2*y-6*y^2+4*y^3)*(2-12*x+12*x^2)'
                 '-(2*y-1)*x*(x-1)*(x^2*(1-x)^2*(2-12*y+12*y^2)-y^2*(1-y)^2*(2-12*x+12*x^2))'
                 '-(1+x*(x-1)*y*(y-1))*(x^2*(1-x)^2*(-12+24*y)-(2*y-6*y^2+4*y^3)*(2-12*x+12*x^2))'
                 '+1-2*x+rho*4*x^3*y^2*(2*y^2-2*y+1)*(y-1)^2*(-1+2*x)*(x-1)^3'
    symbol_names = 'rho'
    symbol_values = '${rho}'
  []
  [forcing_v_deviatoric]
    type = ParsedFunction
    expression = '2*(2*y-1)*x*(x-1)*(2*y-6*y^2+4*y^3)*(2*x-6*x^2+4*x^3)'
                 '+2*(1+x*(x-1)*y*(y-1))*(2-12*y+12*y^2)*(2*x-6*x^2+4*x^3)'
                 '-(2*x-1)*y*(y-1)*(x^2*(1-x)^2*(2-12*y+12*y^2)-y^2*(1-y)^2*(2-12*x+12*x^2))'
                 '-(1+x*(x-1)*y*(y-1))*(-y^2*(1-y)^2*(-12+24*x)+(2*x-6*x^2+4*x^3)*(2-12*y+12*y^2))'
                 '+rho*4*y^3*x^2*(2*x^2-2*x+1)*(x-1)^2*(-1+2*y)*(y-1)^3'
    symbol_names = 'rho'
    symbol_values = '${rho}'
  []
[]

[Executioner]
  type = SIMPLE
  momentum_l_abs_tol = 1e-14
  pressure_l_abs_tol = 1e-14
  momentum_l_max_its = 30
  pressure_l_max_its = 30
  momentum_l_tol = 0.0
  pressure_l_tol = 0.0
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
