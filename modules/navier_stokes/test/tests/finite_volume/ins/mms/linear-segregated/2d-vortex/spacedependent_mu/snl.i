rho = 1.0

advected_interp_method = 'average'
velocity_interp_method = 'rc'

pressure_tag = "pressure_grad"

[GlobalParams]
  rhie_chow_user_object = 'rc'
[]

[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 2
    ny = 2
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
    initial_condition = 1.0
    solver_sys = u_system
    two_term_boundary_expansion = false
  []
  [vel_y]
    type = INSFVVelocityVariable
    initial_condition = 1.0
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
    mu = 'mu'
    momentum_component = 'x'
    complete_expansion = false
    u = vel_x
    v = vel_y
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
    advected_interp_method = ${advected_interp_method}
    velocity_interp_method = ${velocity_interp_method}
    rho = ${rho}
    momentum_component = 'y'
  []
  [v_viscosity]
    type = INSFVMomentumDiffusion
    variable = vel_y
    mu = 'mu'
    momentum_component = 'y'
    complete_expansion = false
    u = vel_x
    v = vel_y
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
  [no-slip-wall-u]
    type = INSFVNoSlipWallBC
    boundary = 'left right top bottom'
    variable = vel_x
    function = '0'
  []
  [no-slip-wall-v]
    type = INSFVNoSlipWallBC
    boundary = 'left right top bottom'
    variable = vel_y
    function = '0'
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
  type = SIMPLENonlinearAssembly
  rhie_chow_user_object = 'rc'
  momentum_systems = 'u_system v_system'
  pressure_system = 'pressure_system'
  pressure_gradient_tag = ${pressure_tag}
  momentum_equation_relaxation = 0.8
  pressure_variable_relaxation = 0.3
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

[Outputs]
  exodus = true
  csv = true
[]

[Postprocessors]
  [h]
    type = AverageElementSize
    outputs = 'console csv'
    execute_on = 'timestep_end'
  []
  [L2u]
    type = ElementL2FunctorError
    approximate = vel_x
    exact = exact_u
    outputs = 'console csv'
    execute_on = 'timestep_end'
  []
  [L2v]
    type = ElementL2FunctorError
    approximate = vel_y
    exact = exact_v
    outputs = 'console csv'
    execute_on = 'timestep_end'
  []
  [L2p]
    approximate = pressure
    exact = exact_p
    type = ElementL2FunctorError
    outputs = 'console csv'
    execute_on = 'timestep_end'
  []
[]
