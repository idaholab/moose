mu = 2.6
rho = 1.0
advected_interp_method = 'average'
walls = 'top bottom'

[Mesh]
  [mesh]
    type = GeneratedMeshGenerator
    dim = 2
    xmin = -2
    xmax = 3
    ymax = 1
    nx = 50
    ny = 10
  []
  [inner_block]
    type = SubdomainBoundingBoxGenerator
    block_id = 1
    bottom_left = '.3 .3 0'
    input = mesh
    top_right = '.7 .7 0'
  []
  [inner_right]
    type = SideSetsBetweenSubdomainsGenerator
    paired_block = 0
    primary_block = 1
    input = inner_block
    new_boundary = fan_outlet
    normal = '1 0 0'
  []
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
    body_force_kernel_names = "u_source;"
    pressure_projection_method = CONSISTENT
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
  [u_time]
    type = LinearFVTimeDerivative
    variable = vel_x
    factor = ${rho}
  []
  [v_time]
    type = LinearFVTimeDerivative
    variable = vel_y
    factor = ${rho}
  []
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
  [u_source]
    type = LinearFVSource
    variable = vel_x
    source_density = 'source'
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
    functor = '1'
  []
  [inlet-v]
    type = LinearFVAdvectionDiffusionFunctorDirichletBC
    boundary = 'left'
    variable = vel_y
    functor = '0.0'
  []
  [walls-u]
    type = LinearFVAdvectionDiffusionFunctorDirichletBC
    boundary = ${walls}
    variable = vel_x
    functor = 0.0
  []
  [walls-v]
    type = LinearFVAdvectionDiffusionFunctorDirichletBC
    boundary = ${walls}
    variable = vel_y
    functor = 0.0
  []
  [walls-p]
    type = LinearFVPressureFluxBC
    Ainv = Ainv
    HbyA_flux = HbyA
    boundary = ${walls}
    variable = pressure
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
  type = PIMPLE
  momentum_l_abs_tol = 1e-8
  pressure_l_abs_tol = 1e-8
  momentum_l_tol = 0
  pressure_l_tol = 0
  rhie_chow_user_object = 'rc'
  momentum_systems = 'u_system v_system'
  pressure_system = 'pressure_system'
  momentum_equation_relaxation = 0.8
  pressure_variable_relaxation = 0.95
  num_iterations = 100
  pressure_absolute_tolerance = 1e-7
  momentum_absolute_tolerance = 1e-7
  momentum_petsc_options_iname = '-pc_type -pc_hypre_type'
  momentum_petsc_options_value = 'hypre boomeramg'
  pressure_petsc_options_iname = '-pc_type -pc_hypre_type'
  pressure_petsc_options_value = 'hypre boomeramg'
  print_fields = false
  num_steps = 50
  num_piso_iterations = 0
  abort_on_solve_fail = true
[]

[Outputs]
  exodus = true
[]

[FunctorMaterials]
  [source_fan]
    type = ParsedFunctorMaterial
    expression = '${rho} * a_x'
    property_name = 'source'
    functor_names = 'a_x'
    block = 1
  []
  [source_null]
    type = GenericFunctorMaterial
    prop_names = 'source'
    prop_values = '0'
    block = 0
  []
[]

[Postprocessors]
  [fan_outlet_integrated_vel_x]
    type = VolumetricFlowRate
    boundary = fan_outlet
    vel_x = vel_x
    vel_y = vel_y
    advected_quantity = 1
    rhie_chow_user_object = 'rc'
  []
  [fan_outlet_area]
    type = AreaPostprocessor
    boundary = fan_outlet
    execute_on = 'initial'
  []
  [average_fan_outlet_vel_x]
    type = ParsedPostprocessor
    expression = 'fan_outlet_integrated_vel_x / fan_outlet_area'
    pp_names = 'fan_outlet_integrated_vel_x fan_outlet_area'
  []
  [inlet_mass]
    type = VolumetricFlowRate
    boundary = left
    vel_x = vel_x
    vel_y = vel_y
    advected_quantity = ${rho}
    rhie_chow_user_object = 'rc'
  []
  [outlet_mass]
    type = VolumetricFlowRate
    boundary = right
    vel_x = vel_x
    vel_y = vel_y
    advected_quantity = ${rho}
    rhie_chow_user_object = 'rc'
  []
  [a_x]
    type = ConstantPostprocessor
    value = 0
  []
[]

[Controls]
  [fan1]
    type = PIDTransientControl
    postprocessor = average_fan_outlet_vel_x
    target = 2
    parameter = 'Postprocessors/a_x/value'
    K_integral = 0
    K_proportional = -1000
    K_derivative = 0
    execute_on = 'initial timestep_begin'
  []
[]
