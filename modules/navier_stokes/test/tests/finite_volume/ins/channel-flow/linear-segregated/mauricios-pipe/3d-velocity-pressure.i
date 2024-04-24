mu = 0.001
rho = 1.0
advected_interp_method = 'average'

[Mesh]
  [mesh]
    type = FileMeshGenerator
    file_name = mesh_pipe_1.e
  []
[]

[Problem]
  linear_sys_names = 'u_system v_system w_system pressure_system'
  previous_nl_solution_required = true
[]

[UserObjects]
  [rc]
    type = RhieChowMassFlux
    u = vel_x
    v = vel_y
    w = vel_z
    pressure = pressure
    rho = ${rho}
    p_diffusion_kernel = p_diffusion
  []
[]

[Variables]
  [vel_x]
    type = MooseLinearVariableFVReal
    initial_condition = 0.1
    solver_sys = u_system
  []
  [vel_y]
    type = MooseLinearVariableFVReal
    solver_sys = v_system
    initial_condition = 0.0
  []
  [vel_z]
    type = MooseLinearVariableFVReal
    solver_sys = w_system
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
    w = vel_z
    momentum_component = 'x'
    rhie_chow_user_object = 'rc'
    use_nonorthogonal_correction = true
  []
  [v_advection_stress]
    type = LinearWCNSFVMomentumFlux
    variable = vel_y
    advected_interp_method = ${advected_interp_method}
    mu = ${mu}
    u = vel_x
    v = vel_y
    w = vel_z
    momentum_component = 'y'
    rhie_chow_user_object = 'rc'
    use_nonorthogonal_correction = true
  []
  [w_advection_stress]
    type = LinearWCNSFVMomentumFlux
    variable = vel_z
    advected_interp_method = ${advected_interp_method}
    mu = ${mu}
    u = vel_x
    v = vel_y
    w = vel_z
    momentum_component = 'z'
    rhie_chow_user_object = 'rc'
    use_nonorthogonal_correction = true
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
  [w_pressure]
    type = LinearFVMomentumPressure
    variable = vel_z
    pressure = pressure
    momentum_component = 'z'
  []
  [p_diffusion]
    type = LinearFVAnisotropicDiffusion
    variable = pressure
    diffusion_tensor = Ainv
    use_nonorthogonal_correction = true
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
    boundary = 'inlet'
    variable = vel_x
    functor = '1.0'
  []
  [inlet-v]
    type = LinearFVAdvectionDiffusionFunctorDirichletBC
    boundary = 'inlet'
    variable = vel_y
    functor = '0.0'
  []
  [inlet-w]
    type = LinearFVAdvectionDiffusionFunctorDirichletBC
    boundary = 'inlet'
    variable = vel_z
    functor = '0.0'
  []
  [walls-u]
    type = LinearFVAdvectionDiffusionFunctorDirichletBC
    boundary = 'wall'
    variable = vel_x
    functor = 0.0
  []
  [walls-v]
    type = LinearFVAdvectionDiffusionFunctorDirichletBC
    boundary = 'wall'
    variable = vel_y
    functor = 0.0
  []
  [walls-w]
    type = LinearFVAdvectionDiffusionFunctorDirichletBC
    boundary = 'wall'
    variable = vel_z
    functor = 0.0
  []
  [outlet_p]
    type = LinearFVAdvectionDiffusionFunctorDirichletBC
    boundary = 'outlet'
    variable = pressure
    functor = 1.4
  []
  [outlet_u]
    type = LinearFVAdvectionDiffusionOutflowBC
    variable = vel_x
    use_two_term_expansion = false
    boundary = 'outlet'
  []
  [outlet_v]
    type = LinearFVAdvectionDiffusionOutflowBC
    variable = vel_y
    use_two_term_expansion = false
    boundary = 'outlet'
  []
  [outlet_w]
    type = LinearFVAdvectionDiffusionOutflowBC
    variable = vel_z
    use_two_term_expansion = false
    boundary = 'outlet'
  []
[]

[Executioner]
  type = LinearSIMPLE
  momentum_l_abs_tol = 1e-9
  pressure_l_abs_tol = 1e-9
  momentum_l_tol = 0
  pressure_l_tol = 0
  rhie_chow_user_object = 'rc'
  momentum_systems = 'u_system v_system w_system'
  pressure_system = 'pressure_system'
  momentum_equation_relaxation = 0.8
  pressure_variable_relaxation = 0.3
  num_iterations = 20
  pressure_absolute_tolerance = 1e-9
  momentum_absolute_tolerance = 1e-9
  # momentum_petsc_options = '-pc_hypre_boomeramg_print_statistics'
  momentum_petsc_options_iname = '-pc_type -pc_hypre_type -pc_hypre_boomeramg_agg_nl -pc_hypre_boomeramg_agg_num_paths -pc_hypre_boomeramg_truncfactor -pc_hypre_boomeramg_strong_threshold -pc_hypre_boomeramg_coarsen_type -pc_hypre_boomeramg_interp_type'
  momentum_petsc_options_value = 'hypre boomeramg 4 1 0.1 0.6 HMIS ext+i'
  # pressure_petsc_options = '-pc_hypre_boomeramg_print_statistics'
  pressure_petsc_options_iname = '-pc_type -pc_hypre_type -pc_hypre_boomeramg_agg_nl -pc_hypre_boomeramg_agg_num_paths -pc_hypre_boomeramg_truncfactor -pc_hypre_boomeramg_strong_threshold -pc_hypre_boomeramg_coarsen_type -pc_hypre_boomeramg_interp_type'
  pressure_petsc_options_value = 'hypre boomeramg 2 1 0.1 0.6 HMIS ext+i'
  print_fields = false
[]

[Outputs]
  exodus = true
[]
