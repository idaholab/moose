mu = 2.6
rho = 1.0
advected_interp_method = 'upwind'
cp = 1000
k = 1

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
  linear_sys_names = 'u_system v_system pressure_system energy_system'
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
  [T_fluid]
    type = MooseLinearVariableFVReal
    solver_sys = energy_system
    initial_condition = 300
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

  [h_advection]
    type = LinearFVEnergyAdvection
    variable = T_fluid
    cp = ${cp}
    advected_interp_method = ${advected_interp_method}
    rhie_chow_user_object = 'rc'
  []
  [conduction]
    type = LinearFVDiffusion
    variable = T_fluid
    diffusion_coeff = ${k}
    use_nonorthogonal_correction = false
  []
  [heat_exchange]
    type = LinearFVVolumetricHeatTransfer
    variable = T_fluid
    h_solid_fluid = 100
    T_fluid = T_fluid
    T_solid = T_solid
    is_solid = false
    block = 1
  []
[]

[FunctorMaterials]
  [constant_functors]
    type = GenericFunctorMaterial
    prop_names = 'cp'
    prop_values = '${cp}'
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
  [inlet_wall_T]
    type = LinearFVAdvectionDiffusionFunctorDirichletBC
    variable = T_fluid
    functor = 300
    boundary = 'left top bottom'
  []
  [outlet_T]
    type = LinearFVAdvectionDiffusionOutflowBC
    variable = T_fluid
    use_two_term_expansion = false
    boundary = right
  []
[]

[AuxVariables]
  [T_solid]
    type = MooseLinearVariableFVReal
    initial_condition = 300
    block = 1
  []
[]

[MultiApps]
  inactive = 'solid'
  [solid]
    type = FullSolveMultiApp
    input_files = solid.i
    execute_on = timestep_begin
    no_restore = true
  []
[]

[Transfers]
  inactive = 'from_solid to_solid'
  [from_solid]
    type = MultiAppGeneralFieldShapeEvaluationTransfer
    from_multi_app = solid
    source_variable = 'T_solid'
    variable = 'T_solid'
    execute_on = timestep_begin
    from_blocks = 1
  []
  [to_solid]
    type = MultiAppGeneralFieldShapeEvaluationTransfer
    to_multi_app = solid
    source_variable = 'T_fluid'
    variable = 'T_fluid'
    execute_on = timestep_begin
    to_blocks = 1
  []
[]

[Executioner]
  type = SIMPLE
  momentum_l_abs_tol = 1e-13
  pressure_l_abs_tol = 1e-13
  energy_l_abs_tol = 1e-13
  momentum_l_tol = 0
  pressure_l_tol = 0
  energy_l_tol = 0
  rhie_chow_user_object = 'rc'
  momentum_systems = 'u_system v_system'
  pressure_system = 'pressure_system'
  energy_system = 'energy_system'
  momentum_equation_relaxation = 0.8
  energy_equation_relaxation = 0.9
  pressure_variable_relaxation = 0.3
  num_iterations = 20
  pressure_absolute_tolerance = 1e-10
  momentum_absolute_tolerance = 1e-10
  energy_absolute_tolerance = 1e-10
  momentum_petsc_options_iname = '-pc_type -pc_hypre_type'
  momentum_petsc_options_value = 'hypre boomeramg'
  pressure_petsc_options_iname = '-pc_type -pc_hypre_type'
  pressure_petsc_options_value = 'hypre boomeramg'
  energy_petsc_options_iname = '-pc_type -pc_hypre_type'
  energy_petsc_options_value = 'hypre boomeramg'
  print_fields = false
  continue_on_max_its = true
[]

[Outputs]
  exodus = true
  execute_on = timestep_end
[]
