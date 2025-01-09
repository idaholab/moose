mu = 2.6
rho = 1.1
advected_interp_method='average'

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

[Mesh]
  [mesh]
    type = CartesianMeshGenerator
    dim = 2
    dx = '0.3'
    dy = '0.3'
    ix = '3'
    iy = '3'
  []
[]

[Problem]
  linear_sys_names = 'u_system v_system pressure_system'
  previous_nl_solution_required = true
[]

[Variables]
  [vel_x]
    type = MooseLinearVariableFVReal
    initial_condition = 0.5
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
    initial_condition = 0.2
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
  [p_diffusion]
    type = LinearFVAnisotropicDiffusion
    variable = pressure
    diffusion_tensor = Ainv
    use_nonorthogonal_correction = false
  []
  [HbyA_divergence]
    type = LinearFVDivergence
    variable = pressure
    face_flux = HbyA
    force_boundary_execution = true
  []
[]

[LinearFVBCs]
  [inlet-u]
    type = LinearFVAdvectionDiffusionFunctorDirichletBC
    boundary = 'left'
    variable = vel_x
    functor = '1.1'
  []
  [inlet-v]
    type = LinearFVAdvectionDiffusionFunctorDirichletBC
    boundary = 'left'
    variable = vel_y
    functor = '0.0'
  []
  [walls-u]
    type = LinearFVAdvectionDiffusionFunctorDirichletBC
    boundary = 'top bottom'
    variable = vel_x
    functor = 0.0
  []
  [walls-v]
    type = LinearFVAdvectionDiffusionFunctorDirichletBC
    boundary = 'top bottom'
    variable = vel_y
    functor = 0.0
  []
  [outlet_p]
    type = LinearFVAdvectionDiffusionFunctorDirichletBC
    boundary = 'right'
    variable = pressure
    functor = 1.4
  []
  [outlet_u]
    type = LinearFVAdvectionDiffusionOutflowBC
    variable = vel_x
    use_two_term_expansion = false
    boundary = right
  []
  [outlet_v]
    type = LinearFVAdvectionDiffusionOutflowBC
    variable = vel_y
    use_two_term_expansion = false
    boundary = right
  []
[]

[Executioner]
  type = SIMPLE
  momentum_l_abs_tol = 1e-10
  pressure_l_abs_tol = 1e-10
  momentum_l_tol = 0
  pressure_l_tol = 0
  rhie_chow_user_object = 'rc'
  momentum_systems = 'u_system v_system'
  pressure_system = 'pressure_system'
  momentum_equation_relaxation = 0.8
  pressure_variable_relaxation = 0.3
  num_iterations = 100
  pressure_absolute_tolerance = 1e-10
  momentum_absolute_tolerance = 1e-10
  momentum_petsc_options_iname = '-pc_type -pc_hypre_type'
  momentum_petsc_options_value = 'hypre boomeramg'
  pressure_petsc_options_iname = '-pc_type -pc_hypre_type'
  pressure_petsc_options_value = 'hypre boomeramg'
  print_fields = false
[]

[Postprocessors]
  [inlet_mass]
    type = VolumetricFlowRate
    boundary = left
    rhie_chow_user_object = rc
    advected_quantity = ${rho}
    vel_x = vel_x
    vel_y = vel_y
    subtract_mesh_velocity = false
  []
  [outlet_mass]
    type = VolumetricFlowRate
    boundary = right
    rhie_chow_user_object = rc
    advected_quantity = ${rho}
    vel_x = vel_x
    vel_y = vel_y
    subtract_mesh_velocity = false
  []
[]

[Outputs]
  csv = true
[]
