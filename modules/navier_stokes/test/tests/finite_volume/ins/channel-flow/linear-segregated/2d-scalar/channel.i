mu = 2.6
rho = 1.0
advected_interp_method = 'upwind'
k1 = 0.1
k2 = 0.2

[Mesh]
  [mesh]
    type = CartesianMeshGenerator
    dim = 2
    dx = '0.25 0.25'
    dy = '0.2'
    ix = '5 5'
    iy = '5'
    subdomain_id = '0 1'
  []
[]

[Problem]
  linear_sys_names = 'u_system v_system pressure_system s1_system s2_system'
  previous_nl_solution_required = true
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
  [scalar1]
    type = MooseLinearVariableFVReal
    solver_sys = s1_system
    initial_condition = 1.1
  []
  [scalar2]
    type = MooseLinearVariableFVReal
    solver_sys = s2_system
    initial_condition = 3
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

  [s1_advection]
    type = LinearFVScalarAdvection
    variable = scalar1
    advected_interp_method = ${advected_interp_method}
    rhie_chow_user_object = 'rc'
  []
  [s1_diffusion]
    type = LinearFVDiffusion
    variable = scalar1
    diffusion_coeff = ${k1}
    use_nonorthogonal_correction = false
  []
  [s2_diffusion]
    type = LinearFVScalarAdvection
    variable = scalar2
    advected_interp_method = ${advected_interp_method}
    rhie_chow_user_object = 'rc'
  []
  [s2_conduction]
    type = LinearFVDiffusion
    variable = scalar2
    diffusion_coeff = ${k2}
    use_nonorthogonal_correction = false
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
  [inlet_wall_scalar1]
    type = LinearFVAdvectionDiffusionFunctorDirichletBC
    variable = scalar1
    functor = 1
    boundary = 'left'
  []
  [outlet_scalar1]
    type = LinearFVAdvectionDiffusionOutflowBC
    variable = scalar1
    use_two_term_expansion = false
    boundary = right
  []
  [inlet_wall_scalar2]
    type = LinearFVAdvectionDiffusionFunctorDirichletBC
    variable = scalar2
    functor = 2
    boundary = 'left'
  []
  [outlet_scalar2]
    type = LinearFVAdvectionDiffusionOutflowBC
    variable = scalar2
    use_two_term_expansion = false
    boundary = right
  []
[]

[Executioner]
  type = SIMPLE
  momentum_l_abs_tol = 1e-13
  pressure_l_abs_tol = 1e-13
  passive_scalar_l_abs_tol = 1e-13
  momentum_l_tol = 0
  pressure_l_tol = 0
  passive_scalar_l_tol = 0
  rhie_chow_user_object = 'rc'
  momentum_systems = 'u_system v_system'
  pressure_system = 'pressure_system'
  passive_scalar_systems = 's1_system s2_system'
  momentum_equation_relaxation = 0.8
  passive_scalar_equation_relaxation = '0.9 0.9'
  pressure_variable_relaxation = 0.3
  # We need to converge the problem to show conservation
  num_iterations = 200
  pressure_absolute_tolerance = 1e-10
  momentum_absolute_tolerance = 1e-10
  passive_scalar_absolute_tolerance = '1e-10 1e-10'
  momentum_petsc_options_iname = '-pc_type -pc_hypre_type'
  momentum_petsc_options_value = 'hypre boomeramg'
  pressure_petsc_options_iname = '-pc_type -pc_hypre_type'
  pressure_petsc_options_value = 'hypre boomeramg'
  passive_scalar_petsc_options_iname = '-pc_type -pc_hypre_type'
  passive_scalar_petsc_options_value = 'hypre boomeramg'
  print_fields = false
  continue_on_max_its = true
[]

[Outputs]
  exodus = true
  execute_on = timestep_end
  hide = 'pressure vel_x vel_y'
[]
