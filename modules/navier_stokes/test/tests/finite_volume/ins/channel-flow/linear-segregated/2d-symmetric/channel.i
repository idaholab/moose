mu = 2.6
rho = 1.2
advected_interp_method = 'upwind'
u_inlet = 1.1
half_width = 0.2

[Mesh]
  [mesh]
    type = CartesianMeshGenerator
    dim = 2
    dx = '1.0'
    dy = '${half_width}'
    ix = '30'
    iy = '15'
    subdomain_id = '0'
  []
  allow_renumbering = false
[]

[Problem]
  linear_sys_names = 'u_system v_system pressure_system'
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
    functor = ${u_inlet}
  []
  [inlet-v]
    type = LinearFVAdvectionDiffusionFunctorDirichletBC
    boundary = 'left'
    variable = vel_y
    functor = '0.0'
  []
  [walls-u]
    type = LinearFVAdvectionDiffusionFunctorDirichletBC
    boundary = 'top'
    variable = vel_x
    functor = 0.0
  []
  [walls-v]
    type = LinearFVAdvectionDiffusionFunctorDirichletBC
    boundary = 'top'
    variable = vel_y
    functor = 0.0
  []
  [symmetry-u]
    type = LinearFVVelocitySymmetryBC
    variable = vel_x
    momentum_component = x
    u = vel_x
    v = vel_y
    boundary = 'bottom'
  []
  [symmetry-v]
    type = LinearFVVelocitySymmetryBC
    variable = vel_y
    momentum_component = y
    u = vel_x
    v = vel_y
    boundary = 'bottom'
  []
  [outlet_u]
    type = LinearFVAdvectionDiffusionOutflowBC
    variable = vel_x
    use_two_term_expansion = false
    boundary = 'right'
  []
  [outlet_v]
    type = LinearFVAdvectionDiffusionOutflowBC
    variable = vel_y
    use_two_term_expansion = false
    boundary = 'right'
  []
  [outlet_p]
    type = LinearFVAdvectionDiffusionFunctorDirichletBC
    boundary = 'right'
    variable = pressure
    functor = 1.4
  []
  [pressure-extrapolation]
    type = LinearFVExtrapolatedPressureBC
    boundary = 'top left'
    variable = pressure
    use_two_term_expansion = true
  []
  [symmetry-p]
    type = LinearFVPressureSymmetryBC
    variable = pressure
    boundary = 'bottom'
    HbyA_flux = 'HbyA' # Functor created in the RhieChowMassFlux UO
  []
[]

[Functions]
  [u_parabolic_profile]
    type = ParsedFunction
    expression = '3/2*${u_inlet}*(1 - pow(y/${half_width}, 2))' # Poiseuille profile
  []
  [u_parabolic_profile_rz]
    type = ParsedFunction
    expression = '2*${u_inlet}*(1 - pow(y/${half_width}, 2))' # Cylindrical profile
  []
[]

[AuxVariables]
  [vel_exact]
    type = MooseLinearVariableFVReal
  []
[]

[AuxKernels]
  [assign_vel_exact]
    type = FunctionAux
    variable = vel_exact
    function = u_parabolic_profile
    execute_on = TIMESTEP_END
  []
[]

[VectorPostprocessors]
  [outlet_velocity_profile]
    type = SideValueSampler
    variable = 'vel_x vel_exact'
    boundary = 'right'
    sort_by = 'y'
    execute_on = TIMESTEP_END
  []
[]

[Executioner]
  type = SIMPLE
  momentum_l_abs_tol = 1e-12
  pressure_l_abs_tol = 1e-12
  momentum_l_tol = 0
  pressure_l_tol = 0
  rhie_chow_user_object = 'rc'
  momentum_systems = 'u_system v_system'
  pressure_system = 'pressure_system'
  momentum_equation_relaxation = 0.5
  pressure_variable_relaxation = 0.3
  num_iterations = 1000
  pressure_absolute_tolerance = 1e-10
  momentum_absolute_tolerance = 1e-10
  momentum_petsc_options_iname = '-pc_type -pc_hypre_type'
  momentum_petsc_options_value = 'hypre boomeramg'
  pressure_petsc_options_iname = '-pc_type -pc_hypre_type'
  pressure_petsc_options_value = 'hypre boomeramg'
  print_fields = false
  continue_on_max_its = true
[]

[Outputs]
  csv = true
  execute_on = timestep_end
[]
