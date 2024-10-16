mu = 1
rho = 1
advected_interp_method = 'min_mod'
pressure_tag = "pressure_grad"

[GlobalParams]
  rhie_chow_user_object = 'rc'
  advected_interp_method = ${advected_interp_method}
  momentum_norm_limiter = true
  velocity_interp_method = 'rc'
  vel_x = 'vel_x'
  vel_y = 'vel_y'
[]

[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 4
    ny = 4
  []
[]

[Problem]
  nl_sys_names = 'u_system v_system pressure_system'
  previous_nl_solution_required = true
[]

[UserObjects]
  [rc]
    type = INSFVRhieChowInterpolatorSegregated
    u = vel_x
    v = vel_y
    pressure = pressure
  []
[]

[Variables]
  [vel_x]
    type = INSFVVelocityVariable
    initial_condition = 1e-6
    solver_sys = u_system
    two_term_boundary_expansion = false
  []
  [vel_y]
    type = INSFVVelocityVariable
    initial_condition = 1e-7
    solver_sys = v_system
    two_term_boundary_expansion = false
  []
  [pressure]
    type = INSFVPressureVariable
    solver_sys = pressure_system
    initial_condition = 1e-8
    two_term_boundary_expansion = true
  []
[]

[FVKernels]
  [u_advection]
    type = INSFVMomentumAdvection
    variable = vel_x
    rho = ${rho}
    momentum_component = 'x'
  []
  [u_viscosity]
    type = INSFVMomentumDiffusion
    variable = vel_x
    mu = ${mu}
    momentum_component = 'x'
  []
  [u_pressure]
    type = INSFVMomentumPressure
    variable = vel_x
    momentum_component = 'x'
    pressure = pressure
    extra_vector_tags = ${pressure_tag}
  []
  [u_forcing]
    type = INSFVBodyForce
    variable = vel_x
    functor = forcing_u
    momentum_component = 'x'
  []

  [v_advection]
    type = INSFVMomentumAdvection
    variable = vel_y
    rho = ${rho}
    momentum_component = 'y'
  []
  [v_viscosity]
    type = INSFVMomentumDiffusion
    variable = vel_y
    mu = ${mu}
    momentum_component = 'y'
  []
  [v_pressure]
    type = INSFVMomentumPressure
    variable = vel_y
    momentum_component = 'y'
    pressure = pressure
    extra_vector_tags = ${pressure_tag}
  []
  [v_forcing]
    type = INSFVBodyForce
    variable = vel_y
    functor = forcing_v
    momentum_component = 'y'
  []

  [p_diffusion]
    type = FVAnisotropicDiffusion
    variable = pressure
    coeff = "Ainv"
    coeff_interp_method = 'average'
  []
  [p_source]
    type = FVDivergence
    variable = pressure
    vector_field = "HbyA"
    force_boundary_execution = true
  []
[]

[FVBCs]
  [no_slip_x]
    type = INSFVNoSlipWallBC
    variable = vel_x
    boundary = 'left right bottom top'
    function = 0
  []
  [no_slip_y]
    type = INSFVNoSlipWallBC
    variable = vel_y
    boundary = 'left right top bottom'
    function = 0
  []
[]

[Executioner]
  type = SIMPLENonlinearAssembly
  rhie_chow_user_object = 'rc'
  momentum_systems = 'u_system v_system'
  pressure_system = 'pressure_system'

  pressure_gradient_tag = ${pressure_tag}
  momentum_equation_relaxation = 0.8
  pressure_variable_relaxation = 0.5
  num_iterations = 2000
  pressure_absolute_tolerance = 1e-8
  momentum_absolute_tolerance = 1e-8
  momentum_petsc_options_iname = '-pc_type -pc_hypre_type'
  momentum_petsc_options_value = 'hypre boomeramg'
  pressure_petsc_options_iname = '-pc_type -pc_hypre_type'
  pressure_petsc_options_value = 'hypre boomeramg'

  momentum_l_abs_tol = 1e-14
  pressure_l_abs_tol = 1e-14
  momentum_l_max_its = 30
  pressure_l_max_its = 30
  momentum_l_tol = 0.0
  pressure_l_tol = 0.0
  print_fields = false

  pin_pressure = true
  pressure_pin_value = 0.25
  pressure_pin_point = '0.5 0.5 0.0'
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
  [forcing_u]
    type = ParsedFunction
    expression = '-4*mu*(-1+2*y)*(y^2-6*x*y^2+6*x^2*y^2-y+6*x*y-6*x^2*y+3*x^2-6*x^3+3*x^4)+1-2*x+rho*4*x^3'
            '*y^2*(2*y^2-2*y+1)*(y-1)^2*(-1+2*x)*(x-1)^3'
    symbol_names = 'mu rho'
    symbol_values = '${mu} ${rho}'
  []
  [forcing_v]
    type = ParsedFunction
    expression = '4*mu*(-1+2*x)*(x^2-6*y*x^2+6*x^2*y^2-x+6*x*y-6*x*y^2+3*y^2-6*y^3+3*y^4)+rho*4*y^3*x^2*(2'
            '*x^2-2*x+1)*(x-1)^2*(-1+2*y)*(y-1)^3'
    symbol_names = 'mu rho'
    symbol_values = '${mu} ${rho}'
  []
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
