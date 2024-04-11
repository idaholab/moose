mu = 2.6
rho = 1.0
advected_interp_method = 'average'

[Mesh]
  [mesh]
    type = CartesianMeshGenerator
    dim = 2
    dx = '0.3'
    dy = '0.3'
    ix = '2'
    iy = '2'
  []
[]

[Problem]
  linear_sys_names = 'u_system v_system pressure_system'
[]

[UserObjects]
  [rc]
    type = RhieChowMassFlux
    u = u
    v = v
    pressure = pressure
    rho = ${rho}
    p_diffusion_kernel = "bazinga"
  []
[]

[Variables]
  [u]
    type = MooseLinearVariableFVReal
    initial_condition = 0.5
    solver_sys = u_system
  []
  [v]
    type = MooseLinearVariableFVReal
    solver_sys = v_system
    initial_condition = 0.0
  []
  [pressure]
    type = MooseLinearVariableFVReal
    solver_sys = pressure_system
    initial_condition = 0.2
  []
[]

[LinearFVKernels]
  [u_advection_stress]
    type = LinearWCNSFVMomentumFlux
    variable = u
    advected_interp_method = ${advected_interp_method}
    mu = ${mu}
    u = u
    v = v
    momentum_component = 'x'
    rhie_chow_user_object = 'rc'
  []
  [v_advection_stress]
    type = LinearWCNSFVMomentumFlux
    variable = v
    advected_interp_method = ${advected_interp_method}
    mu = ${mu}
    u = u
    v = v
    momentum_component = 'y'
    rhie_chow_user_object = 'rc'
  []
  [u_pressure]
    type = LinearFVMomentumPressure
    variable = u
    pressure = pressure
    momentum_component = 'x'
  []
  [v_pressure]
    type = LinearFVMomentumPressure
    variable = v
    pressure = pressure
    momentum_component = 'y'
  []
[]

[LinearFVBCs]
  [inlet-u]
    type = LinearFVAdvectionDiffusionFunctorDirichletBC
    boundary = 'left'
    variable = u
    functor = '1.1'
  []
  [inlet-v]
    type = LinearFVAdvectionDiffusionFunctorDirichletBC
    boundary = 'left'
    variable = v
    functor = '0.0'
  []
  [walls-u]
    type = LinearFVAdvectionDiffusionFunctorDirichletBC
    boundary = 'top bottom'
    variable = u
    functor = 0.0
  []
  [walls-v]
    type = LinearFVAdvectionDiffusionFunctorDirichletBC
    boundary = 'top bottom'
    variable = v
    functor = 0.0
  []
  [outlet_p]
    type = LinearFVAdvectionDiffusionFunctorDirichletBC
    boundary = 'right'
    variable = pressure
    functor = 1.4
  []
[]

[Executioner]
  type = LinearSIMPLE
  momentum_l_abs_tol = 1e-14
  pressure_l_abs_tol = 1e-14
  momentum_l_tol = 0
  pressure_l_tol = 0
  rhie_chow_user_object = 'rc'
  momentum_systems = 'u_system v_system'
  pressure_system = 'pressure_system'
  momentum_equation_relaxation = 0.8
  pressure_variable_relaxation = 0.3
  num_iterations = 1
  pressure_absolute_tolerance = 1e-13
  momentum_absolute_tolerance = 1e-13
  print_fields = true
[]

[Outputs]
  exodus = true
[]
