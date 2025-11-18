mu = 1.2
rho = 1.5
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
  coord_type = 'RZ'
  rz_coord_axis = x
[]

[UserObjects]
  [rc]
    type = RhieChowMassFlux
    u = vel_x
    v = vel_y
    pressure = pressure
    rho = ${rho}
    p_diffusion_kernel = p_diffusion
    pressure_projection_method = consistent
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
    initial_condition = 0.0
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
  [v_viscous_forcing]
    type = LinearFVRZViscousSource
    variable = vel_y
    mu = ${mu}
    momentum_component = 'y'
  []
  [p_diffusion]
    type = LinearFVAnisotropicDiffusion
    variable = pressure
    diffusion_tensor = 'Ainv'
    use_nonorthogonal_correction = false
    use_nonorthogonal_correction_on_boundary = false
  []
  [HbyA_divergence]
    type = LinearFVDivergence
    variable = pressure
    face_flux = 'HbyA'
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
    boundary = 'top left right'
    variable = pressure
    use_two_term_expansion = true
  []
  [pressure-symmetry]
    type = LinearFVPressureSymmetryBC
    boundary = 'bottom'
    variable = pressure
    HbyA_flux = 'HbyA'
  []
[]

[AuxVariables]
  [vel_x_aux]
    type = MooseLinearVariableFVReal
    initial_condition = 0.0
  []
  [vel_y_aux]
    type = MooseLinearVariableFVReal
    initial_condition = 0.0
  []
  [pressure_aux]
    type = MooseLinearVariableFVReal
    initial_condition = 0.0
  []
[]

[AuxKernels]
  [vel_x_function_aux]
    type = FunctionAux
    variable = vel_x_aux
    function = exact_u
    execute_on = TIMESTEP_END
  []
  [vel_y_function_aux]
    type = FunctionAux
    variable = vel_y_aux
    function = exact_v
    execute_on = TIMESTEP_END
  []
  [pressure_function_aux]
    type = FunctionAux
    variable = pressure_aux
    function = exact_p
    execute_on = TIMESTEP_END
  []
[]

[Functions]
  [exact_u]
    type = ParsedFunction
    expression = '4*x^2*y^2 - 20*x^2*y^3 - 8*x^3*y^2 + 36*x^2*y^4 + 40*x^3*y^3 + 4*x^4*y^2 - 28*x^2*y^5 - 72*x^3*y^4 - 20*x^4*y^3 + 8*x^2*y^6 + 56*x^3*y^5 + 36*x^4*y^4 - 16*x^3*y^6 - 28*x^4*y^5 + 8*x^4*y^6'
  []
  [exact_v]
    type = ParsedFunction
    expression = '-2*x*y^3*(x-1)*(2*x-1)*(y-1)^4'
  []
  [exact_p]
    type = ParsedFunction
    expression = 'x*y^2'
  []
  [forcing_u]
    type = ParsedFunction
    symbol_names = 'mu rho'
    symbol_values = '${mu} ${rho}'
    expression = 'rho*(8*x^3*y^4*(x-1)^3*(2*x-1)*(y-1)^6*(4*y^2 - 5*y + 2)) - mu*(4*(y-1)*(72*x^4*y^3 - 103*x^4*y^2 + 41*x^4*y - 4*x^4 - 144*x^3*y^3 + 206*x^3*y^2 - 82*x^3*y + 8*x^3 + 24*x^2*y^5 - 60*x^2*y^4 + 120*x^2*y^3 - 115*x^2*y^2 + 41*x^2*y - 4*x^2 - 24*x*y^5 + 60*x*y^4 - 48*x*y^3 + 12*x*y^2 + 4*y^5 - 10*y^4 + 8*y^3 - 2*y^2)) + y^2'
  []
  [forcing_v]
    type = ParsedFunction
    symbol_names = 'mu rho'
    symbol_values = '${mu} ${rho}'
    expression = 'rho*4*x^2*y^5*(x-1)^2*(y-1)^7*(2*(2*x^2 - 2*x + y) + (2*x - 1)^2*(y - 1)) + mu * (4*y*(2*x-1)*(y-1)^2*(24*x^2*y^2 - 22*x^2*y + 4*x^2 - 24*x*y^2 + 22*x*y - 4*x + 3*y^4 - 6*y^3 + 3*y^2)) + 2*x*y'
  []
[]

[Executioner]
  type = SIMPLE
  momentum_l_abs_tol = 1e-10
  pressure_l_abs_tol = 1e-10
  momentum_l_tol = 0
  pressure_l_tol = 0
  momentum_l_max_its = 500
  pressure_l_max_its = 500
  rhie_chow_user_object = 'rc'
  momentum_systems = 'u_system v_system'
  pressure_system = 'pressure_system'
  momentum_equation_relaxation = 0.85
  pressure_variable_relaxation = 1.0
  num_iterations = 10000
  pressure_absolute_tolerance = 1e-8
  momentum_absolute_tolerance = 1e-8
  momentum_petsc_options_iname = '-pc_type -pc_hypre_type'
  momentum_petsc_options_value = 'hypre boomeramg'
  pressure_petsc_options_iname = '-pc_type -pc_hypre_type'
  pressure_petsc_options_value = 'hypre boomeramg'
  print_fields = false
  continue_on_max_its = true

  pin_pressure = true
  pressure_pin_value = 0.125
  pressure_pin_point = '0.5 0.5 0.0'
[]

[Outputs]
  [csv]
    type = CSV
    execute_on = TIMESTEP_END
  []
[]

[Postprocessors]
  [h]
    type = AverageElementSize
    execute_on = TIMESTEP_END
  []
  [L2u]
    type = ElementL2FunctorError
    approximate = vel_x
    exact = exact_u
    execute_on = TIMESTEP_END
  []
  [L2v]
    type = ElementL2FunctorError
    approximate = vel_y
    exact = exact_v
    execute_on = TIMESTEP_END
  []
  [L2p]
    approximate = pressure
    exact = exact_p
    type = ElementL2FunctorError
    execute_on = TIMESTEP_END
  []
[]
