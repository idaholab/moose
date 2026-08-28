inlet_velocity = 0.1
rho = 998.2
mu = 2e-3
eps_zone_1 = 0.8
eps_zone_2 = 0.6
# eps_zone_1 = 1.0
# eps_zone_2 = 1.0
forch_zone_1 = 30
forch_zone_2 = 30
# forch_zone_1 = 0
# forch_zone_2 = 0
# entry_form_loss = 0 # 20
# corner_form_loss = 0 # 20
# exit_form_loss = 0 # 20
entry_form_loss = 20
corner_form_loss = 20
exit_form_loss = 20
advected_interp_method = 'upwind'

[Mesh]
  [clean_inlet_gmg]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 12
    ny = 12
    xmin = 0
    xmax = 1
    ymin = 0
    ymax = 1
  []
  [clean_inlet_id]
    type = SubdomainIDGenerator
    input = clean_inlet_gmg
    subdomain_id = 1
  []
  [clean_inlet_block]
    type = RenameBlockGenerator
    input = clean_inlet_id
    old_block = '1'
    new_block = 'clean_inlet'
  []
  [clean_inlet_boundary]
    type = RenameBoundaryGenerator
    input = clean_inlet_block
    old_boundary = 'bottom right top left'
    new_boundary = 'bottom_clean clean_to_porous_tmp inner_horizontal_clean inlet'
  []

  [porous_1_patch]
    type = TransfiniteMeshGenerator
    corners = '1 0 0
               3 0 0
               2 1 0
               1 1 0'
    nx = 17
    ny = 13
    bottom_type = LINE
    top_type = LINE
    right_type = LINE
    left_type = LINE
  []
  [porous_1_id]
    type = SubdomainIDGenerator
    input = porous_1_patch
    subdomain_id = 2
  []
  [porous_1_block]
    type = RenameBlockGenerator
    input = porous_1_id
    old_block = '2'
    new_block = 'porous_zone_1'
  []
  [porous_1_boundary]
    type = RenameBoundaryGenerator
    input = porous_1_block
    old_boundary = 'bottom right top left'
    new_boundary = 'bottom_porous diagonal_p_tmp inner_horizontal_porous clean_to_porous_tmp'
  []

  [porous_2_patch]
    type = TransfiniteMeshGenerator
    corners = '2 1 0
               3 0 0
               3 2 0
               2 2 0'
    nx = 13
    ny = 17
    bottom_type = LINE
    top_type = LINE
    right_type = LINE
    left_type = LINE
  []
  [porous_2_id]
    type = SubdomainIDGenerator
    input = porous_2_patch
    subdomain_id = 3
  []
  [porous_2_block]
    type = RenameBlockGenerator
    input = porous_2_id
    old_block = '3'
    new_block = 'porous_zone_2'
  []
  [porous_2_boundary]
    type = RenameBoundaryGenerator
    input = porous_2_block
    old_boundary = 'bottom right top left'
    new_boundary = 'diagonal_p_tmp right_porous porous_to_clean_tmp inner_vertical_porous'
  []

  [clean_outlet_gmg]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 12
    ny = 12
    xmin = 2
    xmax = 3
    ymin = 2
    ymax = 3
  []
  [clean_outlet_id]
    type = SubdomainIDGenerator
    input = clean_outlet_gmg
    subdomain_id = 4
  []
  [clean_outlet_block]
    type = RenameBlockGenerator
    input = clean_outlet_id
    old_block = '4'
    new_block = 'clean_outlet'
  []
  [clean_outlet_boundary]
    type = RenameBoundaryGenerator
    input = clean_outlet_block
    old_boundary = 'bottom right top left'
    new_boundary = 'porous_to_clean_tmp right_clean outlet inner_vertical_clean'
  []

  [stitched_mesh]
    type = StitchMeshGenerator
    inputs = 'clean_inlet_boundary
              porous_1_boundary
              porous_2_boundary
              clean_outlet_boundary'
    stitch_boundaries_pairs = 'clean_to_porous_tmp clean_to_porous_tmp;
                               diagonal_p_tmp diagonal_p_tmp;
                               porous_to_clean_tmp porous_to_clean_tmp'
    clear_stitched_boundary_ids = true
    merge_boundaries_with_same_name = true
  []

  [clean_to_porous]
    type = SideSetsBetweenSubdomainsGenerator
    input = stitched_mesh
    primary_block = 'clean_inlet'
    paired_block = 'porous_zone_1'
    new_boundary = 'clean_to_porous'
  []
  [diagonal_baffle]
    type = SideSetsBetweenSubdomainsGenerator
    input = clean_to_porous
    primary_block = 'porous_zone_1'
    paired_block = 'porous_zone_2'
    new_boundary = 'diagonal_baffle'
  []
  [porous_to_clean]
    type = SideSetsBetweenSubdomainsGenerator
    input = diagonal_baffle
    primary_block = 'porous_zone_2'
    paired_block = 'clean_outlet'
    new_boundary = 'porous_to_clean'
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
    rho = ${rho}
    porosity = porosity
    p_diffusion_kernel = p_diffusion
    pressure_baffle_sidesets = 'clean_to_porous diagonal_baffle porous_to_clean'
    # pressure_gradient_limiter = 'clean_to_porous diagonal_baffle porous_to_clean'
    baffle_form_loss = '${entry_form_loss} ${corner_form_loss} ${exit_form_loss}'
    velocity_form_loss = 'lower_epsilon lower_epsilon lower_epsilon'
    # pressure_gradient_limiter_blend = 0.5
    pressure_baffle_relaxation = 0.2
    debug_baffle = false
    use_flux_velocity_reconstruction = true
    use_reconstructed_pressure_gradient = true
    flux_velocity_reconstruction_relaxation = 1.0
    reconstructed_pressure_gradient_feedback_relaxation = 0.2
    flux_velocity_reconstruction_zero_flux_sidesets = 'bottom_porous inner_horizontal_porous right_porous inner_vertical_porous'
    use_corrected_pressure_gradient = true
  []
[]

[Variables]
  [superficial_u]
    type = MooseLinearVariableFVReal
    solver_sys = u_system
    initial_condition = ${inlet_velocity}
  []
  [superficial_v]
    type = MooseLinearVariableFVReal
    solver_sys = v_system
    initial_condition = 0.0
  []
  [pressure]
    type = MooseLinearVariableFVReal
    solver_sys = pressure_system
    initial_condition = 0.0
  []
[]

[LinearFVKernels]
  [u_advection]
    type = PorousLinearWCNSFVMomentumFlux
    variable = superficial_u
    advected_interp_method = ${advected_interp_method}
    mu = ${mu}
    u = superficial_u
    v = superficial_v
    momentum_component = 'x'
    rhie_chow_user_object = rc
    use_nonorthogonal_correction = true
    porosity_outside_divergence = true
    use_two_point_stress_transmissibility = true
  []
  [v_advection]
    type = PorousLinearWCNSFVMomentumFlux
    variable = superficial_v
    advected_interp_method = ${advected_interp_method}
    mu = ${mu}
    u = superficial_u
    v = superficial_v
    momentum_component = 'y'
    rhie_chow_user_object = rc
    use_nonorthogonal_correction = true
    porosity_outside_divergence = true
    use_two_point_stress_transmissibility = true
  []
  [u_pressure]
    type = LinearFVMomentumPressureUO
    variable = superficial_u
    momentum_component = 'x'
    rhie_chow_user_object = rc
    porosity = porosity
    use_corrected_gradient = true
  []
  [v_pressure]
    type = LinearFVMomentumPressureUO
    variable = superficial_v
    momentum_component = 'y'
    rhie_chow_user_object = rc
    porosity = porosity
    use_corrected_gradient = true
  []
  [u_friction]
    type = LinearFVMomentumPorousFriction
    variable = superficial_u
    Forchheimer_name = forch
    porosity = porosity
    rho = ${rho}
    u = superficial_u
    v = superficial_v
    momentum_component = 'x'
    block = 'porous_zone_1 porous_zone_2'
  []
  [v_friction]
    type = LinearFVMomentumPorousFriction
    variable = superficial_v
    Forchheimer_name = forch
    porosity = porosity
    rho = ${rho}
    u = superficial_u
    v = superficial_v
    momentum_component = 'y'
    block = 'porous_zone_1 porous_zone_2'
  []
  [p_diffusion]
    type = LinearFVAnisotropicDiffusionJump
    variable = pressure
    diffusion_tensor = Ainv
    rhie_chow_user_object = rc
    use_nonorthogonal_correction = true
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
  [inlet_u]
    type = LinearFVAdvectionDiffusionFunctorDirichletBC
    boundary = inlet
    variable = superficial_u
    functor = ${inlet_velocity}
  []
  [inlet_v]
    type = LinearFVAdvectionDiffusionFunctorDirichletBC
    boundary = inlet
    variable = superficial_v
    functor = 0.0
  []

  [outlet_u]
    type = LinearFVAdvectionDiffusionOutflowBC
    boundary = outlet
    variable = superficial_u
    use_two_term_expansion = false
    assume_fully_developed_flow = true
  []
  [outlet_v]
    type = LinearFVAdvectionDiffusionOutflowBC
    boundary = outlet
    variable = superficial_v
    use_two_term_expansion = false
    assume_fully_developed_flow = true
  []
  [outlet_p]
    type = LinearFVAdvectionDiffusionFunctorDirichletBC
    boundary = outlet
    variable = pressure
    functor = 0.0
  []

  [no_slip_u]
    type = LinearFVAdvectionDiffusionFunctorDirichletBC
    boundary = 'bottom_clean inner_horizontal_clean right_clean inner_vertical_clean'
    variable = superficial_u
    functor = 0.0
  []
  [no_slip_v]
    type = LinearFVAdvectionDiffusionFunctorDirichletBC
    boundary = 'bottom_clean inner_horizontal_clean right_clean inner_vertical_clean'
    variable = superficial_v
    functor = 0.0
  []
  [pressure_no_slip]
    type = LinearFVPressureFluxBC
    boundary = 'bottom_clean inner_horizontal_clean right_clean inner_vertical_clean'
    variable = pressure
    HbyA_flux = HbyA
    Ainv = Ainv
  []

  [slip_u]
    type = LinearFVVelocitySymmetryBC
    boundary = 'bottom_porous inner_horizontal_porous right_porous inner_vertical_porous'
    variable = superficial_u
    u = superficial_u
    v = superficial_v
    momentum_component = x
  []
  [slip_v]
    type = LinearFVVelocitySymmetryBC
    boundary = 'bottom_porous inner_horizontal_porous right_porous inner_vertical_porous'
    variable = superficial_v
    u = superficial_u
    v = superficial_v
    momentum_component = y
  []
  [pressure_slip]
    type = LinearFVPressureSymmetryBC
    boundary = 'bottom_porous inner_horizontal_porous right_porous inner_vertical_porous'
    variable = pressure
    HbyA_flux = HbyA
  []
[]

[FunctorMaterials]
  [porosity_clean]
    type = GenericFunctorMaterial
    prop_names = 'porosity'
    prop_values = '1.0'
    block = 'clean_inlet clean_outlet'
  []
  [porosity_zone_1]
    type = GenericFunctorMaterial
    prop_names = 'porosity'
    prop_values = '${eps_zone_1}'
    block = 'porous_zone_1'
  []
  [porosity_zone_2]
    type = GenericFunctorMaterial
    prop_names = 'porosity'
    prop_values = '${eps_zone_2}'
    block = 'porous_zone_2'
  []
  [forch_zone_1]
    type = GenericVectorFunctorMaterial
    prop_names = 'forch'
    prop_values = '${forch_zone_1} ${fparse forch_zone_1} ${forch_zone_1}'
    block = 'porous_zone_1'
  []
  [forch_zone_2]
    type = GenericVectorFunctorMaterial
    prop_names = 'forch'
    prop_values = '${fparse forch_zone_2} ${forch_zone_2} ${forch_zone_2}'
    block = 'porous_zone_2'
  []
[]

[Postprocessors]
  [p_inlet]
    type = SideAverageValue
    variable = pressure
    boundary = inlet
  []
  [p_outlet]
    type = SideAverageValue
    variable = pressure
    boundary = outlet
  []
  [p_total_drop]
    type = ParsedPostprocessor
    expression = 'p_inlet - p_outlet'
    pp_names = 'p_inlet p_outlet'
  []
  [p_clean_in]
    type = ElementAverageValue
    variable = pressure
    block = 'clean_inlet'
  []
  [p_porous_1]
    type = ElementAverageValue
    variable = pressure
    block = 'porous_zone_1'
  []
  [p_porous_2]
    type = ElementAverageValue
    variable = pressure
    block = 'porous_zone_2'
  []
  [p_clean_out]
    type = ElementAverageValue
    variable = pressure
    block = 'clean_outlet'
  []
  [p_entry_drop]
    type = ParsedPostprocessor
    expression = 'p_clean_in - p_porous_1'
    pp_names = 'p_clean_in p_porous_1'
  []
  [p_corner_drop]
    type = ParsedPostprocessor
    expression = 'p_porous_1 - p_porous_2'
    pp_names = 'p_porous_1 p_porous_2'
  []
  [p_exit_drop]
    type = ParsedPostprocessor
    expression = 'p_porous_2 - p_clean_out'
    pp_names = 'p_porous_2 p_clean_out'
  []
[]

[VectorPostprocessors]
  [p_inlet_leg]
    type = LineValueSampler
    variable = pressure
    warn_discontinuous_face_values = false
    start_point = '0.05 0.56 0'
    end_point = '2.40 0.56 0'
    num_points = 201
    sort_by = x
  []
  [p_outlet_leg]
    type = LineValueSampler
    variable = pressure
    warn_discontinuous_face_values = false
    start_point = '2.55 0.05 0'
    end_point = '2.55 2.95 0'
    num_points = 240
    sort_by = y
  []
  [p_diagonal]
    type = LineValueSampler
    variable = pressure
    warn_discontinuous_face_values = false
    start_point = '2.04 0.94 0'
    end_point = '2.94 0.04 0'
    num_points = 101
    sort_by = x
  []
[]

[AuxVariables]
  [porosity_aux]
    type = MooseLinearVariableFVReal
  []
    [superficial_w]
    type = MooseLinearVariableFVReal
  []
[]

[AuxKernels]
  [porosity_display]
    type = FunctorAux
    variable = porosity_aux
    functor = porosity
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
  momentum_equation_relaxation = 0.7
  pressure_variable_relaxation = 0.2
  num_iterations = 2000
  pressure_absolute_tolerance = 1e-8
  momentum_absolute_tolerance = 1e-8
  momentum_petsc_options_iname = '-pc_type -pc_hypre_type'
  momentum_petsc_options_value = 'hypre boomeramg'
  pressure_petsc_options_iname = '-pc_type -pc_hypre_type'
  pressure_petsc_options_value = 'hypre boomeramg'
  continue_on_max_its = true
[]

[Outputs]
  exodus = true
  csv = true
  execute_on = timestep_end
[]
