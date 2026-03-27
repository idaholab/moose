length_left = 5.0
length_right = 5.0
height = 1.0

nx_left = 25
nx_right = 25
ny = 3

epsilon_left = 1.0
epsilon_right = 0.5
u_in = 1.0
p_out = 0.0
mu = 1.0

density_factor = 1e-3

advected_interp_method = 'average'

[Mesh]
  [mesh]
    type = CartesianMeshGenerator
    dim = 2

    # Two blocks are enough for this verification problem: one block on each side of the
    # internal porous-baffle interface.
    dx = '${length_left} ${length_right}'
    dy = '${height}'
    ix = '${nx_left} ${nx_right}'
    iy = '${ny}'
    subdomain_id = '1 2'
  []
  [baffle]
    type = SideSetsBetweenSubdomainsGenerator
    input = mesh
    primary_block = '1'
    paired_block = '2'
    new_boundary = 'baffle'
  []
  [split_top_bottom]
    type = BreakBoundaryOnSubdomainGenerator
    input = baffle
    boundaries = 'top bottom'
  []
  [delete]
    type = BoundaryDeletionGenerator
    boundary_names = 'top bottom'
    input = split_top_bottom
  []
[]

[Problem]
  linear_sys_names = 'u_system v_system pressure_system'
  previous_nl_solution_required = true
[]

[UserObjects]
  [rc]
    type = PorousRhieChowMassFlux
    u = superficial_u
    v = superficial_v
    pressure = pressure
    rho = 'rho_aux'
    porosity = 'porosity'
    p_diffusion_kernel = p_diffusion

    pressure_baffle_sidesets = 'baffle'
    pressure_baffle_relaxation = 0.1

    debug_baffle = false

    # use_flux_velocity_reconstruction = true
    # use_reconstructed_pressure_gradient = true
    flux_velocity_reconstruction_relaxation = 1.0
    flux_velocity_reconstruction_zero_flux_sidesets = 'top_to_1 top_to_2 bottom_to_1 bottom_to_2'

    use_interpolated_density_in_bernoulli_jump = true
    # pressure_gradient_limiter = 'baffle'
    # pressure_gradient_limiter_blend = 1.0
    use_corrected_pressure_gradient = true
  []
[]

[Variables]
  [superficial_u]
    type = MooseLinearVariableFVReal
    solver_sys = u_system
    initial_condition = ${u_in}
  []
  [superficial_v]
    type = MooseLinearVariableFVReal
    solver_sys = v_system
    initial_condition = 0.0
  []
  [pressure]
    type = MooseLinearVariableFVReal
    solver_sys = pressure_system
    initial_condition = ${p_out}
  []
[]

[LinearFVKernels]
  [u_advection]
    type = PorousLinearWCNSFVMomentumFlux
    variable = superficial_u
    advected_interp_method = ${advected_interp_method}
    mu = 'mu'
    u = superficial_u
    v = superficial_v
    momentum_component = 'x'
    rhie_chow_user_object = rc
    use_nonorthogonal_correction = false
    porosity_outside_divergence = true
    use_two_point_stress_transmissibility = false
  []
  [v_advection]
    type = PorousLinearWCNSFVMomentumFlux
    variable = superficial_v
    advected_interp_method = ${advected_interp_method}
    mu = 'mu'
    u = superficial_u
    v = superficial_v
    momentum_component = 'y'
    rhie_chow_user_object = rc
    use_nonorthogonal_correction = false
    porosity_outside_divergence = true
    use_two_point_stress_transmissibility = false
  []
  [u_pressure]
    type = LinearFVMomentumPressureUO
    variable = superficial_u
    momentum_component = 'x'
    rhie_chow_user_object = rc
    porosity = 'porosity'
    use_corrected_gradient = true
  []
  [v_pressure]
    type = LinearFVMomentumPressureUO
    variable = superficial_v
    momentum_component = 'y'
    rhie_chow_user_object = rc
    porosity = 'porosity'
    use_corrected_gradient = true
  []
  [p_diffusion]
    type = LinearFVAnisotropicDiffusionJump
    variable = pressure
    diffusion_tensor = Ainv
    rhie_chow_user_object = rc
    use_nonorthogonal_correction = false
    debug_baffle_jump = false
  []
  [HbyA_divergence]
    type = LinearFVDivergence
    variable = pressure
    face_flux = HbyA
    force_boundary_execution = true
  []
[]

[LinearFVBCs]
  [left_u]
    type = LinearFVAdvectionDiffusionFunctorDirichletBC
    boundary = left
    variable = superficial_u
    functor = ${u_in}
  []
  [left_v]
    type = LinearFVAdvectionDiffusionFunctorDirichletBC
    boundary = left
    variable = superficial_v
    functor = 0.0
  []

  [pressure-extrapolation]
    type = LinearFVExtrapolatedPressureBC
    boundary = 'left'
    variable = pressure
    use_two_term_expansion = true
  []

  # Fix the outlet pressure and leave the outlet velocity free.
  [outlet_u]
    type = LinearFVAdvectionDiffusionOutflowBC
    boundary = right
    variable = superficial_u
    use_two_term_expansion = true
    assume_fully_developed_flow = false
    # assume_fully_developed_flow = true
  []
  [outlet_v]
    type = LinearFVAdvectionDiffusionOutflowBC
    boundary = right
    variable = superficial_v
    use_two_term_expansion = true
    assume_fully_developed_flow = false
    # assume_fully_developed_flow = true
  []
  [outlet_p]
    type = LinearFVAdvectionDiffusionFunctorDirichletBC
    boundary = right
    variable = pressure
    functor = ${p_out}
  []

  # Symmetry removes any wall losses and keeps the exact solution one-dimensional.
  [symmetry_u]
    type = LinearFVVelocitySymmetryBC
    boundary = 'top_to_1 top_to_2 bottom_to_1 bottom_to_2'
    variable = superficial_u
    u = superficial_u
    v = superficial_v
    momentum_component = x
  []
  [symmetry_v]
    type = LinearFVVelocitySymmetryBC
    boundary = 'top_to_1 top_to_2 bottom_to_1 bottom_to_2'
    variable = superficial_v
    u = superficial_u
    v = superficial_v
    momentum_component = y
  []
  [pressure_symmetric]
    type = LinearFVPressureSymmetryBC
    boundary = 'top_to_1 top_to_2 bottom_to_1 bottom_to_2'
    variable = pressure
    HbyA_flux = 'HbyA'
  []
[]

[Functions]
  [rho]
    type = ParsedFunction
    expression = '(1e3 - 0.05e3 * x)*${density_factor}'
    # expression = '1'
  []
[]

[FunctorMaterials]
  [porosity]
    type = PiecewiseByBlockFunctorMaterial
    prop_name = porosity
    subdomain_to_prop_value = '1 ${epsilon_left} 2 ${epsilon_right}'
  []
  [mu]
    type = ParsedFunctorMaterial
    property_name = mu
    expression = '${mu}'
  []
[]

[AuxVariables]
  [rho_aux]
    type = MooseLinearVariableFVReal
  []
  [porosity_aux]
    type = MooseLinearVariableFVReal
  []
[]

[AuxKernels]
  [assign_rho_aux]
    type = FunctorAux
    variable = rho_aux
    functor = 'rho'
    execute_on = 'initial timestep_end'
  []
  [assign_porosity_aux]
    type = FunctorAux
    variable = porosity_aux
    functor = 'porosity'
    execute_on = 'initial timestep_end'
  []
[]

[Postprocessors]
  # Overall pressure drop from the solved field.
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
  [delta_p]
    type = ParsedPostprocessor
    expression = 'p_left - p_right'
    pp_names = 'p_left p_right'
  []

  # The block averages give a more direct measure of the interface jump because the exact
  # solution is piecewise constant in each block.
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

  # Check the superficial velocities on both sides of the jump.
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
[]

[VectorPostprocessors]
  [centerline_solution]
    type = LineValueSampler
    variable = 'rho_aux superficial_u pressure'
    # start_point = '${fparse length_left - 0.15} 0.5 0'
    # end_point = '${fparse length_left + 0.15} 0.5 0'
    start_point = '0.1 0.5 0'
    end_point = '${fparse length_left + length_right -0.1} 0.5 0'
    num_points = 50
    sort_by = x
    execute_on = timestep_end
  []
[]

[Executioner]
  type = SIMPLE
  momentum_l_abs_tol = 1e-14
  pressure_l_abs_tol = 1e-14
  momentum_l_tol = 0
  pressure_l_tol = 0
  rhie_chow_user_object = rc
  momentum_systems = 'u_system v_system'
  pressure_system = pressure_system
  momentum_equation_relaxation = 0.4
  pressure_variable_relaxation = 0.2
  num_iterations = 2000
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
  exodus = true
  csv = true
  execute_on = timestep_end
[]
