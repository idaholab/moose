# Axisymmetric pipe version of ../1d-simple-channel/porous-baffle-1d.i: the same axial flow in an
# RZ pipe of radius 0.5 with symmetry conditions on the axis and on the wall (slip), so the
# solution is independent of r and must reproduce the one-dimensional results. The comments below
# are those of the one-dimensional input.
# One-dimensional verification of Bernoulli jumps, porous drag, and form loss.
# Mass conservation gives u_block_1 = u_block_2 = 0.1 and u_block_jump = 0.
# Without drag or form loss, the two pressure jumps (right minus left) are 0.03 and -0.03,
# so p_left = p_right = p_jump = 0. With mu = 1 and Forchheimer = 50, the porous block
# adds a pressure drop of 0.25. A form-loss coefficient of 10 adds 0.4 at each baffle,
# giving p_jump = p_left - p_right = 1.05 and baffle jumps of -0.37 and -0.43.

mu = 0.0 # 1e-2
rho = 2.0
advected_interp_method = 'average'
forchheimer_value = 50
inlet_u = 0.1

[FVGradientMethods]
  [jump_aware]
    type = FVPressureJumpGreenGaussGradient
  []
  [velocity_jump_aware]
    type = FVVelocityJumpGreenGaussGradient
  []
  [reconstructed]
    type = FVReconstructedPressureGradient
    base_gradient_method = jump_aware
    gradient_relaxation = 1.0
  []
[]

[Mesh]
  [mesh]
    type = CartesianMeshGenerator
    dim = 2
    dx = '0.5 0.5 0.5'
    ix = '200 200 200'
    dy = '0.5'
    iy = '2'
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
  coord_type = RZ
  rz_coord_axis = X
[]

[Problem]
  linear_sys_names = 'u_system v_system pressure_system'
  previous_nl_solution_required = true
[]

[UserObjects]
  [pressure_jump]
    type = BernoulliFormLossPressureJump
    boundary = 'baffle baffle2'
    porosity = porosity
    density = ${rho}
  []
  [rc]
    type = PorousRhieChowMassFlux
    u = superficial_u
    v = superficial_v
    pressure = pressure
    rho = ${rho}
    porosity = porosity
    p_diffusion_kernel = p_diffusion
    pressure_jump_models = pressure_jump
    pressure_jump_relaxation = 0.1
  []
[]

[Variables]
  [superficial_u]
    type = MooseLinearVariableFVReal
    solver_sys = u_system
    initial_condition = ${inlet_u}
    gradient_method = velocity_jump_aware
  []
  [superficial_v]
    type = MooseLinearVariableFVReal
    solver_sys = v_system
    initial_condition = 0
    gradient_method = velocity_jump_aware
  []
  [pressure]
    type = MooseLinearVariableFVReal
    solver_sys = pressure_system
    initial_condition = 0.0
    gradient_method = jump_aware
  []
[]

[FVInterpolationMethods]
  [average]
    type = FVGeometricAverage
  []
[]

[LinearFVKernels]
  [u_advection]
    type = LinearPWCNSFVMomentumFlux
    variable = superficial_u
    advected_interp_method_name = ${advected_interp_method}
    mu = ${mu}
    u = superficial_u
    v = superficial_v
    momentum_component = 'x'
    rhie_chow_user_object = rc
    use_nonorthogonal_correction = false
  []
  [v_advection]
    type = LinearPWCNSFVMomentumFlux
    variable = superficial_v
    advected_interp_method_name = ${advected_interp_method}
    mu = ${mu}
    u = superficial_u
    v = superficial_v
    momentum_component = 'y'
    rhie_chow_user_object = rc
    use_nonorthogonal_correction = false
  []
  [u_pressure]
    type = LinearPWCNSFVMomentumPressure
    variable = superficial_u
    momentum_component = 'x'
    porosity = porosity
    pressure = pressure
    gradient_method = reconstructed
  []
  [v_pressure]
    type = LinearPWCNSFVMomentumPressure
    variable = superficial_v
    momentum_component = 'y'
    porosity = porosity
    pressure = pressure
    gradient_method = reconstructed
  []
  [u_friction]
    type = LinearFVMomentumFriction
    variable = superficial_u
    Forchheimer_name = forch
    porosity = porosity
    rho = ${rho}
    u = superficial_u
    v = superficial_v
    momentum_component = 'x'
    block = 2
  []
  [v_friction]
    type = LinearFVMomentumFriction
    variable = superficial_v
    Forchheimer_name = forch
    porosity = porosity
    rho = ${rho}
    u = superficial_u
    v = superficial_v
    momentum_component = 'y'
    block = 2
  []
  [p_diffusion]
    type = LinearFVPressureCorrectionDiffusionJump
    variable = pressure
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
  [inlet_v]
    type = LinearFVAdvectionDiffusionFunctorDirichletBC
    boundary = left
    variable = superficial_v
    functor = 0
  []
  [outlet_u]
    type = LinearFVAdvectionDiffusionOutflowBC
    boundary = right
    variable = superficial_u
    use_two_term_expansion = false
  []
  [outlet_v]
    type = LinearFVAdvectionDiffusionOutflowBC
    boundary = right
    variable = superficial_v
    use_two_term_expansion = false
  []
  # The axis (bottom) and the slip wall (top) are both symmetry planes for this flow.
  [symmetry_u]
    type = LinearFVVelocitySymmetryBC
    boundary = 'bottom top'
    variable = superficial_u
    u = superficial_u
    v = superficial_v
    momentum_component = 'x'
  []
  [symmetry_v]
    type = LinearFVVelocitySymmetryBC
    boundary = 'bottom top'
    variable = superficial_v
    u = superficial_u
    v = superficial_v
    momentum_component = 'y'
  []
  [symmetry_p]
    type = LinearFVPressureSymmetryBC
    boundary = 'bottom top'
    variable = pressure
    HbyA_flux = HbyA
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
  # is 0.5 / 200 = 0.0025, so the adjacent centers are offset by half that distance. The radial
  # coordinate 0.375 is the center of the outer row of cells.
  [p_baffle_1_left]
    type = PointValue
    variable = pressure
    point = '0.49875 0.375 0'
  []
  [p_baffle_1_right]
    type = PointValue
    variable = pressure
    point = '0.50125 0.375 0'
  []
  [p_baffle_2_left]
    type = PointValue
    variable = pressure
    point = '0.99875 0.375 0'
  []
  [p_baffle_2_right]
    type = PointValue
    variable = pressure
    point = '1.00125 0.375 0'
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
  momentum_systems = 'u_system v_system'
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
