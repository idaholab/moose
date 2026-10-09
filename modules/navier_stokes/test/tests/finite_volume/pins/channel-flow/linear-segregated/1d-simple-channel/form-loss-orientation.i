# Isolated form-loss orientation check. Equal porosity removes the reversible Bernoulli jump,
# so the remaining pressure drop is the irreversible form loss of 0.1.

mu = 0.0
rho = 2.0
advected_interp_method = 'average'
inlet_u = 0.1
form_loss = 10
primary_block = 1
paired_block = 2
velocity_inlet_boundary = left
pressure_outlet_boundary = right
direction = 1

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
    dx = '0.5 0.5'
    ix = '8 8'
    subdomain_id = '1 2'
  []
  [baffle]
    type = SideSetsBetweenSubdomainsGenerator
    input = mesh
    primary_block = ${primary_block}
    paired_block = ${paired_block}
    new_boundary = 'baffle'
  []
[]

[Problem]
  linear_sys_names = 'u_system pressure_system'
  previous_nl_solution_required = true
[]

[UserObjects]
  [pressure_jump]
    type = BernoulliFormLossPressureJump
    boundary = 'baffle'
    porosity = porosity
    density = ${rho}
    form_loss = ${form_loss}
  []
  [rc]
    type = PorousRhieChowMassFlux
    u = superficial_u
    pressure = pressure
    rho = ${rho}
    porosity = porosity
    p_diffusion_kernel = p_diffusion
    pressure_jump_models = pressure_jump
    pressure_jump_relaxation = 1.0
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
    boundary = ${velocity_inlet_boundary}
    variable = superficial_u
    functor = ${inlet_u}
  []
  [outlet_u]
    type = LinearFVAdvectionDiffusionOutflowBC
    boundary = ${pressure_outlet_boundary}
    variable = superficial_u
    use_two_term_expansion = false
  []
  [outlet_p]
    type = LinearFVAdvectionDiffusionFunctorDirichletBC
    boundary = ${pressure_outlet_boundary}
    variable = pressure
    functor = 0.0
  []
[]

[FunctorMaterials]
  [porosity]
    type = GenericFunctorMaterial
    prop_names = porosity
    prop_values = 1
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
  [directed_pressure_loss]
    type = ParsedPostprocessor
    expression = '${direction} * (p_left - p_right)'
    pp_names = 'p_left p_right'
  []
  [u_avg]
    type = ElementAverageValue
    variable = superficial_u
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
  num_iterations = 200
  pressure_absolute_tolerance = 1e-10
  momentum_absolute_tolerance = 1e-8
  momentum_petsc_options_iname = '-pc_type -pc_hypre_type'
  momentum_petsc_options_value = 'hypre boomeramg'
  pressure_petsc_options_iname = '-pc_type -pc_hypre_type'
  pressure_petsc_options_value = 'hypre boomeramg'
[]

[Outputs]
  csv = true
  execute_on = 'timestep_end'
[]
