mu = 0.0 # 1e-2
rho = 2.0
advected_interp_method = 'average'
forchheimer_value = 50
inlet_u = 0.1

[FVGradientMethods]
  [reconstructed]
    type = FVReconstructedPressureGradient
    base_gradient_method = green-gauss
    gradient_relaxation = 1.0
  []
[]

[Mesh]
  [mesh]
    type = CartesianMeshGenerator
    dim = 1
    dx = '0.5 0.5 0.5'
    ix = '200 200 200'
    subdomain_id = '1 2 3'
  []
  [baffle]
    type = SideSetsBetweenSubdomainsGenerator
    input = mesh
    primary_block = '1'
    paired_block = '2'
    new_boundary = 'baffle'
  []
  [baffle2]
    type = SideSetsBetweenSubdomainsGenerator
    input = baffle
    primary_block = '2'
    paired_block = '3'
    new_boundary = 'baffle2'
  []
[]

[Problem]
  linear_sys_names = 'u_system pressure_system'
  previous_nl_solution_required = true
[]

[UserObjects]
  [rc]
    type = PorousRhieChowMassFlux
    u = superficial_u
    pressure = pressure
    rho = ${rho}
    porosity = porosity
    p_diffusion_kernel = p_diffusion
    pressure_baffle_sidesets = 'baffle baffle2'
    pressure_gradient_limiter = 'baffle baffle2'
    pressure_baffle_relaxation = 0.1
  []
[]

[Variables]
  [superficial_u]
    type = MooseLinearVariableFVReal
    solver_sys = u_system
    initial_condition = ${inlet_u}
  []
  [pressure]
    type = MooseLinearVariableFVReal
    solver_sys = pressure_system
    initial_condition = 0.0
    gradient_method = reconstructed
  []
[]

[FVInterpolationMethods]
  [average]
    type = FVGeometricAverage
  []
[]

[LinearFVKernels]
  [u_advection]
    type = PorousLinearWCNSFVMomentumFlux
    variable = superficial_u
    advected_interp_method_name = ${advected_interp_method}
    mu = ${mu}
    u = superficial_u
    momentum_component = 'x'
    rhie_chow_user_object = rc
    use_nonorthogonal_correction = false
    use_two_point_stress_transmissibility = true
  []
  [u_pressure]
    type = LinearFVMomentumPressure
    variable = superficial_u
    momentum_component = 'x'
    porosity = porosity
    pressure = pressure
  []
  [u_friction]
    type = LinearFVMomentumFriction
    variable = superficial_u
    Forchheimer_name = forch
    porosity = porosity
    rho = ${rho}
    u = superficial_u
    momentum_component = 'x'
    block = 2
  []
  [p_diffusion]
    type = LinearFVAnisotropicDiffusionJump
    variable = pressure
    diffusion_tensor = Ainv
    rhie_chow_user_object = rc
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
  [inlet_u]
    type = LinearFVAdvectionDiffusionFunctorDirichletBC
    boundary = left
    variable = superficial_u
    functor = ${inlet_u}
  []
  [outlet_u]
    type = LinearFVAdvectionDiffusionOutflowBC
    boundary = right
    variable = superficial_u
    use_two_term_expansion = false
  []
  [outlet_p]
    type = LinearFVAdvectionDiffusionFunctorDirichletBC
    boundary = right
    variable = pressure
    functor = 0.0
  []
[]

[FunctorMaterials]
  [forch]
    type = GenericVectorFunctorMaterial
    prop_names = forch
    prop_values = '${forchheimer_value} ${forchheimer_value} ${forchheimer_value}'
  []
  [porosity]
    type = PiecewiseByBlockFunctorMaterial
    prop_name = porosity
    subdomain_to_prop_value = '1 0.5 2 1.0 3 0.5'
  []
[]

[Postprocessors]
  [p_left]
    type = SideAverageValue
    variable = pressure
    boundary = left
  []
  [p_right]
    type = SideAverageValue
    variable = pressure
    boundary = right
  []
  [p_jump]
    type = ParsedPostprocessor
    expression = 'p_left - p_right'
    pp_names = 'p_left p_right'
  []
  # These points are the centers of the cells adjacent to x = 0.5 and x = 1.0. The mesh spacing
  # is 0.5 / 200 = 0.0025, so the adjacent centers are offset by half that distance.
  [p_baffle_1_left]
    type = PointValue
    variable = pressure
    point = '0.49875 0 0'
  []
  [p_baffle_1_right]
    type = PointValue
    variable = pressure
    point = '0.50125 0 0'
  []
  [p_baffle_2_left]
    type = PointValue
    variable = pressure
    point = '0.99875 0 0'
  []
  [p_baffle_2_right]
    type = PointValue
    variable = pressure
    point = '1.00125 0 0'
  []
  [p_baffle_1_jump]
    type = ParsedPostprocessor
    expression = 'p_baffle_1_right - p_baffle_1_left'
    pp_names = 'p_baffle_1_right p_baffle_1_left'
  []
  [p_baffle_2_jump]
    type = ParsedPostprocessor
    expression = 'p_baffle_2_right - p_baffle_2_left'
    pp_names = 'p_baffle_2_right p_baffle_2_left'
  []
  [u_block_1]
    type = ElementAverageValue
    variable = superficial_u
    block = 1
  []
  [u_block_2]
    type = ElementAverageValue
    variable = superficial_u
    block = 2
  []
  [u_block_jump]
    type = ParsedPostprocessor
    expression = 'u_block_1 - u_block_2'
    pp_names = 'u_block_1 u_block_2'
  []
[]

[AuxVariables]
  [porosity_aux]
    type = MooseLinearVariableFVReal
  []
[]

[AuxKernels]
  [por]
    type = FunctorAux
    variable = porosity_aux
    functor = porosity
    execute_on = 'timestep_end'
  []
[]

[Executioner]
  type = SIMPLE
  momentum_l_abs_tol = 1e-12
  pressure_l_abs_tol = 1e-12
  momentum_l_tol = 0
  pressure_l_tol = 0
  rhie_chow_user_object = rc
  momentum_systems = 'u_system'
  pressure_system = pressure_system
  momentum_equation_relaxation = 0.3
  pressure_variable_relaxation = 0.1
  num_iterations = 1000
  pressure_absolute_tolerance = 1e-8
  # The Forchheimer case plateaus near 1e-2 after pressure and velocity stabilize; 2e-2 stops
  # with the total pressure drop within 2e-6 relative to the value after 1000 iterations.
  momentum_absolute_tolerance = 2e-2
  momentum_petsc_options_iname = '-pc_type -pc_hypre_type'
  momentum_petsc_options_value = 'hypre boomeramg'
  pressure_petsc_options_iname = '-pc_type -pc_hypre_type'
  pressure_petsc_options_value = 'hypre boomeramg'
[]

[Outputs]
  csv = true
  execute_on = 'timestep_end'
[]
