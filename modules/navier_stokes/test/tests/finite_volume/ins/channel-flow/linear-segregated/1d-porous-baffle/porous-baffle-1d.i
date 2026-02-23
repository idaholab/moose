mu = 0.0
rho = 2.0
advected_interp_method = 'upwind'

[Mesh]
  [mesh]
    type = CartesianMeshGenerator
    dim = 1
    dx = '0.5 0.5'
    ix = '10 10'
    subdomain_id = '1 2'
  []
  [baffle]
    type = SideSetsBetweenSubdomainsGenerator
    input = mesh
    primary_block = '1'
    paired_block = '2'
    new_boundary = 'baffle'
  []
  # [baffle2]
  #   type = SideSetsBetweenSubdomainsGenerator
  #   input = baffle
  #   primary_block = '2'
  #   paired_block = '3'
  #   new_boundary = 'baffle2'
  # []
[]

[Problem]
  linear_sys_names = 'u_system pressure_system'
  previous_nl_solution_required = true
[]

[UserObjects]
  [rc]
    type = RhieChowMassFlux
    u = superficial_u
    pressure = pressure
    rho = ${rho}
    porosity = porosity
    p_diffusion_kernel = p_diffusion
    pressure_baffle_sidesets = 'baffle'
    pressure_baffle_relaxation = 0.5
    debug_baffle = true
    use_flux_velocity_reconstruction = false
    flux_velocity_reconstruction_relaxation = 1.0
    use_corrected_pressure_gradient = true
  []
[]

[Variables]
  [superficial_u]
    type = MooseLinearVariableFVReal
    solver_sys = u_system
    initial_condition = 0.1
  []
  [pressure]
    type = MooseLinearVariableFVReal
    solver_sys = pressure_system
    initial_condition = 0.0
  []
[]

[LinearFVKernels]
  [u_advection]
    type = LinearWCNSFVMomentumFlux
    variable = superficial_u
    advected_interp_method = ${advected_interp_method}
    mu = ${mu}
    u = superficial_u
    momentum_component = 'x'
    rhie_chow_user_object = rc
    use_nonorthogonal_correction = false
    porosity_outside_divergence = true # keep global default
  []
  [u_pressure]
    type = LinearFVMomentumPressureUO
    variable = superficial_u
    momentum_component = 'x'
    rhie_chow_user_object = rc
    porosity = porosity
    use_corrected_gradient = true
  []
  [p_diffusion]
    type = LinearFVAnisotropicDiffusionJump
    variable = pressure
    diffusion_tensor = Ainv
    rhie_chow_user_object = rc
    use_nonorthogonal_correction = false
    debug_baffle_jump = true
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
    functor = 0.1
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
    subdomain_to_prop_value = '1 0.5 2 1.0'
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
  [p_block_1]
    type = ElementAverageValue
    variable = pressure
    block = 1
  []
  [p_block_2]
    type = ElementAverageValue
    variable = pressure
    block = 2
  []
  [p_block_jump]
    type = ParsedPostprocessor
    expression = 'p_block_1 - p_block_2'
    pp_names = 'p_block_1 p_block_2'
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

[VectorPostprocessors]
  [u_line]
    type = LineValueSampler
    variable = superficial_u
    start_point = '0 0 0'
    end_point = '1 0 0'
    num_points = 41
    sort_by = id
  []
  [p_line]
    type = LineValueSampler
    variable = pressure
    start_point = '0 0 0'
    end_point = '1 0 0'
    num_points = 41
    sort_by = id
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
  num_iterations = 500
  pressure_absolute_tolerance = 1e-8
  momentum_absolute_tolerance = 1e-8
  momentum_petsc_options_iname = '-pc_type -pc_hypre_type'
  momentum_petsc_options_value = 'hypre boomeramg'
  pressure_petsc_options_iname = '-pc_type -pc_hypre_type'
  pressure_petsc_options_value = 'hypre boomeramg'
  # print_fields = true
  continue_on_max_its = true
[]

[Outputs]
  exodus = true
  csv = true
  execute_on = 'timestep_end'
[]
