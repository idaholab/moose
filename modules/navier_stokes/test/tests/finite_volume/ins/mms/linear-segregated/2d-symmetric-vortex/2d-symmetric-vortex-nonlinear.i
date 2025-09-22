mu = 1.1
rho = 1.2
advected_interp_method = 'average'
velocity_interp_method = 'rc'

pressure_tag = "pressure_grad"

[Problem]
  nl_sys_names = 'u_system v_system pressure_system'
  previous_nl_solution_required = true
[]

[GlobalParams]
  rhie_chow_user_object = 'rc'
[]

[UserObjects]
  [rc]
    type = INSFVRhieChowInterpolatorSegregated
    u = vel_x
    v = vel_y
    pressure = pressure
  []
[]

[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 2
    ny = 2
  []
[]

[Variables]
  [vel_x]
    type = INSFVVelocityVariable
    initial_condition = 0.0
    solver_sys = u_system
    two_term_boundary_expansion = false
  []
  [vel_y]
    type = INSFVVelocityVariable
    initial_condition = 0.0
    solver_sys = v_system
    two_term_boundary_expansion = false
  []
  [pressure]
    type = INSFVPressureVariable
    solver_sys = pressure_system
    initial_condition = 0.0
    two_term_boundary_expansion = false
  []
[]

[FVKernels]
  [u_advection]
    type = INSFVMomentumAdvection
    variable = vel_x
    advected_interp_method = ${advected_interp_method}
    velocity_interp_method = ${velocity_interp_method}
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
    type = FVBodyForce
    variable = vel_x
    function = forcing_u
  []
  [v_advection]
    type = INSFVMomentumAdvection
    variable = vel_y
    advected_interp_method = ${advected_interp_method}
    velocity_interp_method = ${velocity_interp_method}
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
  [v_forcing]
    type = FVBodyForce
    variable = vel_y
    function = forcing_v
  []
[]

[FVBCs]
  [inlet-u]
    type = INSFVInletVelocityBC
    boundary = 'left right top'
    variable = vel_x
    function = 'exact_u'
  []
  [inlet-v]
    type = INSFVInletVelocityBC
    boundary = 'left right top'
    variable = vel_y
    function = 'exact_v'
  []
  [axis-u]
    type = INSFVSymmetryVelocityBC
    boundary = 'bottom'
    variable = vel_x
    u = vel_x
    v = vel_y
    mu = ${mu}
    momentum_component = x
  []
  [axis-v]
    type = INSFVSymmetryVelocityBC
    boundary = 'bottom'
    variable = vel_y
    u = vel_x
    v = vel_y
    mu = ${mu}
    momentum_component = y
  []
  [axis-p]
    type = INSFVSymmetryPressureBC
    boundary = 'bottom'
    variable = pressure
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
  type = SIMPLENonlinearAssembly
  momentum_l_abs_tol = 1e-14
  pressure_l_abs_tol = 1e-14
  momentum_l_tol = 0
  pressure_l_tol = 0
  rhie_chow_user_object = 'rc'
  momentum_systems = 'u_system v_system'
  pressure_system = 'pressure_system'
  pressure_gradient_tag = ${pressure_tag}
  momentum_equation_relaxation = 0.7
  pressure_variable_relaxation = 0.3
  num_iterations = 1000
  pressure_absolute_tolerance = 1e-8
  momentum_absolute_tolerance = 1e-8
  print_fields = false

  pin_pressure = true
  pressure_pin_value = 0.25
  pressure_pin_point = '0.5 0.5 0.0'
[]

[Outputs]
  exodus = true
  csv = false
  perf_graph = false
  print_nonlinear_residuals = false
  print_linear_residuals = true
[]

[Postprocessors]
  [h]
    type = AverageElementSize
    # outputs = 'csv'
    execute_on = TIMESTEP_END
  []
  [L2u]
    type = ElementL2FunctorError
    approximate = vel_x
    exact = exact_u
    # outputs = 'csv'
    execute_on = TIMESTEP_END
  []
  [L2v]
    type = ElementL2FunctorError
    approximate = vel_y
    exact = exact_v
    # outputs = 'csv'
    execute_on = TIMESTEP_END
  []
  [L2p]
    approximate = pressure
    exact = exact_p
    type = ElementL2FunctorError
    # outputs = 'csv'
    execute_on = TIMESTEP_END
  []
[]

