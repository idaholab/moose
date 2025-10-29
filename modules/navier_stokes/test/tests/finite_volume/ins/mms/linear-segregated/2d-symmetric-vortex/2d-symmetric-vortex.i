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
[]

[UserObjects]
  [rc]
    type = RhieChowMassFlux
    u = vel_x
    v = vel_y
    pressure = pressure
    rho = ${rho}
    p_diffusion_kernel = p_diffusion
    body_force_kernel_names = "u_forcing; v_forcing"
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
  [p_diffusion]
    type = LinearFVAnisotropicDiffusion
    variable = pressure
    diffusion_tensor = 'Ainv' # Functor created in the RhieChowMassFlux UO
    use_nonorthogonal_correction = false
    use_nonorthogonal_correction_on_boundary = false
  []
  [HbyA_divergence]
    type = LinearFVDivergence
    variable = pressure
    face_flux = 'HbyA' # Functor created in the RhieChowMassFlux UO
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
    type = LinearFVPressureFluxBC
    boundary = 'top left right'
    variable = pressure
    HbyA_flux = HbyA
    Ainv = Ainv
  []
  [pressure-symmetry]
    type = LinearFVPressureSymmetryBC
    boundary = 'bottom'
    variable = pressure
    HbyA_flux = 'HbyA' # Functor created in the RhieChowMassFlux UO
  []
[]

[Functions]
  [exact_u]
    type = ParsedFunction
    expression = 'x^2*(1-x)^2*(8*y^3 - 9*y^2 + 1)'
  []
  [exact_v]
    type = ParsedFunction
    expression = '-(2*x - 6*x^2 + 4*x^3)*y*(1-y)^2*(2*y+1)'
  []
  [exact_p]
    type = ParsedFunction
    expression = 'y^2'
  []
  [forcing_u]
    type = ParsedFunction
    symbol_names = 'mu rho'
    symbol_values = '${mu} ${rho}'
    expression = 'rho*( (x^2*(1-x)^2)*(2*x - 6*x^2 + 4*x^3)*
                        ( (8*y^3 - 9*y^2 + 1)^2
                          - (2*y^4 - 3*y^3 + y)*(24*y^2 - 18*y) ) )
                  - mu*( (2 - 12*x + 12*x^2)*(8*y^3 - 9*y^2 + 1)
                         + (x^2*(1-x)^2)*(48*y - 18) )'
  []
  [forcing_v]
    type = ParsedFunction
    symbol_names = 'mu rho'
    symbol_values = '${mu} ${rho}'
    expression = 'rho*( (8*y^3 - 9*y^2 + 1)*(2*y^4 - 3*y^3 + y)*
                        ( (2*x - 6*x^2 + 4*x^3)^2
                          - (x^2*(1-x)^2)*(2 - 12*x + 12*x^2) ) )
                  + mu*( (24*x - 12)*(2*y^4 - 3*y^3 + y)
                         + (2*x - 6*x^2 + 4*x^3)*(24*y^2 - 18*y) )
                  + 2*y'
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
  num_iterations = 10000
  pressure_absolute_tolerance = 1e-7
  momentum_absolute_tolerance = 1e-7
  momentum_petsc_options_iname = '-pc_type -pc_hypre_type'
  momentum_petsc_options_value = 'hypre boomeramg'
  pressure_petsc_options_iname = '-pc_type -pc_hypre_type'
  pressure_petsc_options_value = 'hypre boomeramg'
  print_fields = false
  continue_on_max_its = true

  pin_pressure = true
  pressure_pin_value = 0.25
  pressure_pin_point = '0.5 0.5 0.0'
[]

[Outputs]
  exodus = true
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
