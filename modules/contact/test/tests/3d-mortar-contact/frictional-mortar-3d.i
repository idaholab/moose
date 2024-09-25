starting_point = 0.25
offset = 0.00

[GlobalParams]
  displacements = 'disp_x disp_y disp_z'
  volumetric_locking_correction = true
[]

[AuxVariables]
  [mortar_tangent_x]
    family = LAGRANGE
    order = FIRST
  []
  [mortar_tangent_y]
    family = LAGRANGE
    order = FIRST
  []
  [mortar_tangent_z]
    family = LAGRANGE
    order = FIRST
  []
[]

[AuxKernels]
  [friction_x_component]
   type = MortarFrictionalPressureVectorAux
   primary_boundary = 'bottom_top'
   secondary_boundary = 'top_bottom'
   tangent_one = mortar_tangential_lm
   tangent_two = mortar_tangential_3d_lm
   variable = mortar_tangent_x
   component = 0
   boundary = 'top_bottom'
  []
  [friction_y_component]
   type = MortarFrictionalPressureVectorAux
   primary_boundary = 'bottom_top'
   secondary_boundary = 'top_bottom'
   tangent_one = mortar_tangential_lm
   tangent_two = mortar_tangential_3d_lm
   variable = mortar_tangent_y
   component = 1
   boundary = 'top_bottom'
  []
  [friction_z_component]
   type = MortarFrictionalPressureVectorAux
   primary_boundary = 'bottom_top'
   secondary_boundary = 'top_bottom'
   tangent_one = mortar_tangential_lm
   tangent_two = mortar_tangential_3d_lm
   variable = mortar_tangent_z
   component = 2
   boundary = 'top_bottom'
  []
[]

[Mesh]
  [top_block]
    type = GeneratedMeshGenerator
    dim = 3
    nx = 3
    ny = 3
    nz = 3
    xmin = -0.25
    xmax = 0.25
    ymin = -0.25
    ymax = 0.25
    zmin = -0.25
    zmax = 0.25
    elem_type = HEX8
  []
  [rotate_top_block]
    type = TransformGenerator
    input = top_block
    transform = ROTATE
    vector_value = '0 0 0'
  []
  [top_block_sidesets]
    type = RenameBoundaryGenerator
    input = rotate_top_block
    old_boundary = '0 1 2 3 4 5'
    new_boundary = 'top_bottom top_back top_right top_front top_left top_top'
  []
  [top_block_id]
    type = SubdomainIDGenerator
    input = top_block_sidesets
    subdomain_id = 1
  []
  [bottom_block]
    type = GeneratedMeshGenerator
    dim = 3
    nx = 10
    ny = 10
    nz = 2
    xmin = -.5
    xmax = .5
    ymin = -.5
    ymax = .5
    zmin = -.3
    zmax = -.25
    elem_type = HEX8
  []
  [bottom_block_id]
    type = SubdomainIDGenerator
    input = bottom_block
    subdomain_id = 2
  []
  [bottom_block_change_boundary_id]
    type = RenameBoundaryGenerator
    input = bottom_block_id
    old_boundary = '0 1 2 3 4 5'
    new_boundary = '100 101 102 103 104 105'
  []
  [combined]
    type = MeshCollectionGenerator
    inputs = 'top_block_id bottom_block_change_boundary_id'
  []
  [block_rename]
    type = RenameBlockGenerator
    input = combined
    old_block = '1 2'
    new_block = 'top_block bottom_block'
  []
  [bottom_right_sideset]
    type = SideSetsAroundSubdomainGenerator
    input = block_rename
    new_boundary = bottom_right
    block = bottom_block
    normal = '1 0 0'
  []
  [bottom_left_sideset]
    type = SideSetsAroundSubdomainGenerator
    input = bottom_right_sideset
    new_boundary = bottom_left
    block = bottom_block
    normal = '-1 0 0'
  []
  [bottom_top_sideset]
    type = SideSetsAroundSubdomainGenerator
    input = bottom_left_sideset
    new_boundary = bottom_top
    block = bottom_block
    normal = '0 0 1'
  []
  [bottom_bottom_sideset]
    type = SideSetsAroundSubdomainGenerator
    input = bottom_top_sideset
    new_boundary = bottom_bottom
    block = bottom_block
    normal = '0  0 -1'
  []
  [bottom_front_sideset]
    type = SideSetsAroundSubdomainGenerator
    input = bottom_bottom_sideset
    new_boundary = bottom_front
    block = bottom_block
    normal = '0 1 0'
  []
  [bottom_back_sideset]
    type = SideSetsAroundSubdomainGenerator
    input = bottom_front_sideset
    new_boundary = bottom_back
    block = bottom_block
    normal = '0 -1 0'
  []
  [secondary]
    input = bottom_back_sideset
    type = LowerDBlockFromSidesetGenerator
    sidesets = 'top_bottom' # top_back top_left'
    new_block_id = '10001'
    new_block_name = 'secondary_lower'
  []
  [primary]
    input = secondary
    type = LowerDBlockFromSidesetGenerator
    sidesets = 'bottom_top'
    new_block_id = '10000'
    new_block_name = 'primary_lower'
  []
  uniform_refine = 0
  allow_renumbering = false
[]

[Variables]
  [mortar_normal_lm]
    block = 'secondary_lower'
    use_dual = true
  []
  [mortar_tangential_lm]
    block = 'secondary_lower'
    use_dual = true
  []
  [mortar_tangential_3d_lm]
    block = 'secondary_lower'
    use_dual = true
  []
[]

[Physics/SolidMechanics/QuasiStatic]
  [all]
    add_variables = true
    strain = FINITE
    block = '1 2'
    use_automatic_differentiation = false
    generate_output = 'stress_xx stress_xy stress_xz stress_yy stress_zz'
  []
[]

[Materials]
  [tensor]
    type = ComputeIsotropicElasticityTensor
    block = '1'
    youngs_modulus = 1.0e4
    poissons_ratio = 0.0
  []
  [stress]
    type = ComputeFiniteStrainElasticStress
    block = '1'
  []

  [tensor_1000]
    type = ComputeIsotropicElasticityTensor
    block = '2'
    youngs_modulus = 1e5
    poissons_ratio = 0.0
  []
  [stress_1000]
    type = ComputeFiniteStrainElasticStress
    block = '2'
  []
[]

[UserObjects]
  [weighted_vel_uo]
    type = LMWeightedVelocitiesUserObject
    primary_boundary = 'bottom_top'
    secondary_boundary = 'top_bottom'
    primary_subdomain = 'primary_lower'
    secondary_subdomain = 'secondary_lower'
    lm_variable_normal = mortar_normal_lm
    lm_variable_tangential_one = mortar_tangential_lm
    lm_variable_tangential_two = mortar_tangential_3d_lm
    secondary_variable = disp_x
    disp_x = disp_x
    disp_y = disp_y
    disp_z = disp_z
  []
[]

[Constraints]
  [friction]
    type = ComputeFrictionalForceLMMechanicalContact
    primary_boundary = 'bottom_top'
    secondary_boundary = 'top_bottom'
    primary_subdomain = 'primary_lower'
    secondary_subdomain = 'secondary_lower'
    variable = mortar_normal_lm
    disp_x = disp_x
    disp_y = disp_y
    disp_z = disp_z
    use_displaced_mesh = true
    mu = 0.4
    c = 1e4
    c_t = 1.0e4
    friction_lm = mortar_tangential_lm
    friction_lm_dir = mortar_tangential_3d_lm
    weighted_gap_uo = weighted_vel_uo
    weighted_velocities_uo = weighted_vel_uo
  []
  [normal_x]
    type = NormalMortarMechanicalContact
    primary_boundary = 'bottom_top'
    secondary_boundary = 'top_bottom'
    primary_subdomain = 'primary_lower'
    secondary_subdomain = 'secondary_lower'
    variable = mortar_normal_lm
    secondary_variable = disp_x
    component = x
    use_displaced_mesh = true
    compute_lm_residuals = false
    weighted_gap_uo = weighted_vel_uo
  []
  [normal_y]
    type = NormalMortarMechanicalContact
    primary_boundary = 'bottom_top'
    secondary_boundary = 'top_bottom'
    primary_subdomain = 'primary_lower'
    secondary_subdomain = 'secondary_lower'
    variable = mortar_normal_lm
    secondary_variable = disp_y
    component = y
    use_displaced_mesh = true
    compute_lm_residuals = false
    weighted_gap_uo = weighted_vel_uo
  []
  [normal_z]
    type = NormalMortarMechanicalContact
    primary_boundary = 'bottom_top'
    secondary_boundary = 'top_bottom'
    primary_subdomain = 'primary_lower'
    secondary_subdomain = 'secondary_lower'
    variable = mortar_normal_lm
    secondary_variable = disp_z
    component = z
    use_displaced_mesh = true
    compute_lm_residuals = false
    weighted_gap_uo = weighted_vel_uo
  []
  [tangential_x]
    type = TangentialMortarMechanicalContact
    primary_boundary = 'bottom_top'
    secondary_boundary = 'top_bottom'
    primary_subdomain = 'primary_lower'
    secondary_subdomain = 'secondary_lower'
    variable = mortar_tangential_lm
    secondary_variable = disp_x
    component = x
    use_displaced_mesh = true
    compute_lm_residuals = false
    weighted_velocities_uo = weighted_vel_uo
  []
  [tangential_y]
    type = TangentialMortarMechanicalContact
    primary_boundary = 'bottom_top'
    secondary_boundary = 'top_bottom'
    primary_subdomain = 'primary_lower'
    secondary_subdomain = 'secondary_lower'
    variable = mortar_tangential_lm
    secondary_variable = disp_y
    component = y
    use_displaced_mesh = true
    compute_lm_residuals = false
    weighted_velocities_uo = weighted_vel_uo
  []
  [tangential_z]
    type = TangentialMortarMechanicalContact
    primary_boundary = 'bottom_top'
    secondary_boundary = 'top_bottom'
    primary_subdomain = 'primary_lower'
    secondary_subdomain = 'secondary_lower'
    variable = mortar_tangential_lm
    secondary_variable = disp_z
    component = z
    use_displaced_mesh = true
    compute_lm_residuals = false
    weighted_velocities_uo = weighted_vel_uo
  []
  [tangential_dir_x]
    type = TangentialMortarMechanicalContact
    primary_boundary = 'bottom_top'
    secondary_boundary = 'top_bottom'
    primary_subdomain = 'primary_lower'
    secondary_subdomain = 'secondary_lower'
    variable = mortar_tangential_3d_lm
    secondary_variable = disp_x
    component = x
    direction = direction_2
    use_displaced_mesh = true
    compute_lm_residuals = false
    weighted_velocities_uo = weighted_vel_uo
  []
  [tangential_dir_y]
    type = TangentialMortarMechanicalContact
    primary_boundary = 'bottom_top'
    secondary_boundary = 'top_bottom'
    primary_subdomain = 'primary_lower'
    secondary_subdomain = 'secondary_lower'
    variable = mortar_tangential_3d_lm
    secondary_variable = disp_y
    component = y
    direction = direction_2
    use_displaced_mesh = true
    compute_lm_residuals = false
    weighted_velocities_uo = weighted_vel_uo
  []
  [tangential_dir_z]
    type = TangentialMortarMechanicalContact
    primary_boundary = 'bottom_top'
    secondary_boundary = 'top_bottom'
    primary_subdomain = 'primary_lower'
    secondary_subdomain = 'secondary_lower'
    variable = mortar_tangential_3d_lm
    secondary_variable = disp_z
    component = z
    direction = direction_2
    use_displaced_mesh = true
    compute_lm_residuals = false
    weighted_velocities_uo = weighted_vel_uo
  []
[]

[BCs]
  [botx]
    type = DirichletBC
    variable = disp_x
    boundary = 'bottom_left bottom_right bottom_front bottom_back'
    value = 0.0
  []
  [boty]
    type = DirichletBC
    variable = disp_y
    boundary = 'bottom_left bottom_right bottom_front bottom_back'
    value = 0.0
  []
  [botz]
    type = DirichletBC
    variable = disp_z
    boundary = 'bottom_left bottom_right bottom_front bottom_back'
    value = 0.0
  []
  [topx]
    type = DirichletBC
    variable = disp_x
    boundary = 'top_top'
    value = 0.0
  []
  [topy]
    type = DirichletBC
    variable = disp_y
    boundary = 'top_top'
    value = 0.0
  []
  [topz]
    type = FunctionDirichletBC
    variable = disp_z
    boundary = 'top_top'
    function = '-${starting_point} * sin(2 * pi / 40 * t) + ${offset}'
  []
[]

[Executioner]
  type = Transient
  end_time = .025
  dt = .025
  dtmin = .001
  solve_type = 'PJFNK'
  petsc_options = '-snes_converged_reason -ksp_converged_reason -snes_ksp_ew'
  petsc_options_iname = '-pc_type -pc_factor_mat_solver_type -pc_factor_shift_type -pc_factor_shift_amount -mat_mffd_err'
  petsc_options_value = 'lu       superlu_dist                  NONZERO               1e-14                  1e-5'
  l_max_its = 15
  nl_max_its = 30
  nl_rel_tol = 1e-11
  nl_abs_tol = 1e-12
  line_search = 'basic'
[]

[Debug]
  show_var_residual_norms = true
[]

[Outputs]
  exodus = true
  csv = true
[]

[Preconditioning]
  [smp]
    type = SMP
    full = true
  []
[]

[Postprocessors]
  active = 'contact'
  [contact]
    type = ContactDOFSetSize
    variable = mortar_normal_lm
    subdomain = 'secondary_lower'
    execute_on = 'nonlinear timestep_end'
  []
[]

[VectorPostprocessors]
  [contact-pressure]
    type = NodalValueSampler
    block = secondary_lower
    variable = mortar_normal_lm
    sort_by = 'id'
    execute_on = NONLINEAR
  []
  [frictional-pressure]
    type = NodalValueSampler
    block = secondary_lower
    variable = mortar_tangential_lm
    sort_by = 'id'
    execute_on = NONLINEAR
  []
  [frictional-pressure-3d]
    type = NodalValueSampler
    block = secondary_lower
    variable = mortar_tangential_3d_lm
    sort_by = 'id'
    execute_on = NONLINEAR
  []
  [tangent_x]
    type = NodalValueSampler
    block = secondary_lower
    variable = mortar_tangent_x
    sort_by = 'id'
    execute_on = NONLINEAR
  []
  [tangent_y]
    type = NodalValueSampler
    block = secondary_lower
    variable = mortar_tangent_y
    sort_by = 'id'
    execute_on = NONLINEAR
  []
[]
