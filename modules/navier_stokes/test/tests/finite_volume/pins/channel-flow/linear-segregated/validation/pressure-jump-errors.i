# Compact fixture for constructor and setup errors on pressure-jump objects.

mu = 0.0
rho = 2.0
advected_interp_method = 'average'
inlet_u = 0.1

[FVGradientMethods]
  [jump_aware]
    type = FVPressureJumpGreenGaussGradient
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
    dim = 1
    dx = '0.5 0.5 0.5'
    ix = '2 2 2'
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
  active = 'pressure_jump rc'
  [pressure_jump]
    type = BernoulliFormLossPressureJump
    boundary = 'baffle baffle2'
    porosity = porosity
    density = ${rho}
  []
  [duplicate_jump]
    type = BernoulliFormLossPressureJump
    boundary = 'baffle'
    porosity = porosity
    density = ${rho}
  []
  [rc]
    type = PorousRhieChowMassFlux
    u = superficial_u
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
  active = 'u_advection u_pressure p_diffusion HbyA_divergence'
  [u_advection]
    type = LinearPWCNSFVMomentumFlux
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
    type = LinearPWCNSFVMomentumPressure
    variable = superficial_u
    momentum_component = 'x'
    porosity = porosity
    pressure = pressure
    gradient_method = reconstructed
  []
  [p_diffusion]
    type = LinearFVPressureCorrectionDiffusionJump
    variable = pressure
    diffusion_tensor = Ainv
    rhie_chow_user_object = rc
    use_nonorthogonal_correction = false
  []
  [ordinary_p_diffusion]
    type = LinearFVPressureCorrectionDiffusion
    variable = pressure
    diffusion_tensor = Ainv
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
  [porosity]
    type = PiecewiseByBlockFunctorMaterial
    prop_name = porosity
    subdomain_to_prop_value = '1 0.5 2 1.0 3 0.5'
  []
[]

[Executioner]
  type = SIMPLE
  rhie_chow_user_object = rc
  momentum_systems = 'u_system'
  pressure_system = pressure_system
  num_iterations = 1
  continue_on_max_its = true
[]

[Outputs]
  csv = false
  exodus = false
[]
