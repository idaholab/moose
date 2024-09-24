starting_point = 0.25
offset = 0.00

[GlobalParams]
  displacements = 'disp_x disp_y disp_z'
  volumetric_locking_correction = true
[]

[AuxVariables]
  [penalty_normal_pressure]
    order = FIRST
    family = LAGRANGE
  []
  [penalty_frictional_pressure_one]
    order = FIRST
    family = LAGRANGE
  []
  [accumulated_slip_one]
    order = FIRST
    family = LAGRANGE
  []
  [penalty_frictional_pressure_two]
    order = FIRST
    family = LAGRANGE
  []
  [accumulated_slip_two]
    order = FIRST
    family = LAGRANGE
  []
[]

[Problem]
  type = AugmentedLagrangianContactFEProblem
[]

[AuxKernels]
  [penalty_normal_pressure_auxk]
    type = PenaltyMortarUserObjectAux
    variable = penalty_normal_pressure
    user_object = friction_uo
    contact_quantity = normal_pressure
  []
  [penalty_frictional_pressure_one_auxk]
    type = PenaltyMortarUserObjectAux
    variable = penalty_frictional_pressure_one
    user_object = friction_uo
    contact_quantity = tangential_pressure_one
  []
  [penalty_accumulated_slip_auxk]
    type = PenaltyMortarUserObjectAux
    variable = accumulated_slip_one
    user_object = friction_uo
    contact_quantity = accumulated_slip_one
  []
  [penalty_frictional_pressure_two_auxk]
    type = PenaltyMortarUserObjectAux
    variable = penalty_frictional_pressure_two
    user_object = friction_uo
    contact_quantity = tangential_pressure_two
  []
  [penalty_accumulated_slip_two_auxk]
    type = PenaltyMortarUserObjectAux
    variable = accumulated_slip_two
    user_object = friction_uo
    contact_quantity = accumulated_slip_two
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
  allow_renumbering = false
[]

[Variables]

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
    youngs_modulus = 1.0e5
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

# Other object should mix formulations
[UserObjects]
  [friction_uo]
    type = PenaltyFrictionUserObject
    primary_boundary = 'bottom_top'
    secondary_boundary = 'top_bottom'
    primary_subdomain = 'primary_lower'
    secondary_subdomain = 'secondary_lower'
    disp_x = disp_x
    disp_y = disp_y
    disp_z = disp_z
    friction_coefficient = 0.4
    secondary_variable = disp_x
    penalty = 1e0
    penalty_friction = 1e1

    slip_tolerance = 7.0e-4 # 1e-6
    penetration_tolerance = 7.0e-4
    # max_penalty_multiplier = 10
    penalty_multiplier = 10
    penalty_multiplier_friction = 5
  []
[]

[Constraints]
  [normal_x]
    type = NormalMortarMechanicalContact
    primary_boundary = 'bottom_top'
    secondary_boundary = 'top_bottom'
    primary_subdomain = 'primary_lower'
    secondary_subdomain = 'secondary_lower'
    secondary_variable = disp_x
    component = x
    use_displaced_mesh = true
    compute_lm_residuals = false
    weighted_gap_uo = friction_uo
  []
  [normal_y]
    type = NormalMortarMechanicalContact
    primary_boundary = 'bottom_top'
    secondary_boundary = 'top_bottom'
    primary_subdomain = 'primary_lower'
    secondary_subdomain = 'secondary_lower'
    secondary_variable = disp_y
    component = y
    use_displaced_mesh = true
    compute_lm_residuals = false
    weighted_gap_uo = friction_uo
  []
  [normal_z]
    type = NormalMortarMechanicalContact
    primary_boundary = 'bottom_top'
    secondary_boundary = 'top_bottom'
    primary_subdomain = 'primary_lower'
    secondary_subdomain = 'secondary_lower'
    secondary_variable = disp_z
    component = z
    use_displaced_mesh = true
    compute_lm_residuals = false
    weighted_gap_uo = friction_uo
  []
  [tangential_x]
    type = TangentialMortarMechanicalContact
    primary_boundary = 'bottom_top'
    secondary_boundary = 'top_bottom'
    primary_subdomain = 'primary_lower'
    secondary_subdomain = 'secondary_lower'
    secondary_variable = disp_x
    component = x
    use_displaced_mesh = true
    compute_lm_residuals = false
    weighted_velocities_uo = friction_uo
  []
  [tangential_y]
    type = TangentialMortarMechanicalContact
    primary_boundary = 'bottom_top'
    secondary_boundary = 'top_bottom'
    primary_subdomain = 'primary_lower'
    secondary_subdomain = 'secondary_lower'
    secondary_variable = disp_y
    component = y
    use_displaced_mesh = true
    compute_lm_residuals = false
    weighted_velocities_uo = friction_uo
  []
  [tangential_z]
    type = TangentialMortarMechanicalContact
    primary_boundary = 'bottom_top'
    secondary_boundary = 'top_bottom'
    primary_subdomain = 'primary_lower'
    secondary_subdomain = 'secondary_lower'
    secondary_variable = disp_z
    component = z
    use_displaced_mesh = true
    compute_lm_residuals = false
    weighted_velocities_uo = friction_uo
  []
  [tangential_dir_x]
    type = TangentialMortarMechanicalContact
    primary_boundary = 'bottom_top'
    secondary_boundary = 'top_bottom'
    primary_subdomain = 'primary_lower'
    secondary_subdomain = 'secondary_lower'
    secondary_variable = disp_x
    component = x
    direction = direction_2
    use_displaced_mesh = true
    compute_lm_residuals = false
    weighted_velocities_uo = friction_uo
  []
  [tangential_dir_y]
    type = TangentialMortarMechanicalContact
    primary_boundary = 'bottom_top'
    secondary_boundary = 'top_bottom'
    primary_subdomain = 'primary_lower'
    secondary_subdomain = 'secondary_lower'
    secondary_variable = disp_y
    component = y
    direction = direction_2
    use_displaced_mesh = true
    compute_lm_residuals = false
    weighted_velocities_uo = friction_uo
  []
  [tangential_dir_z]
    type = TangentialMortarMechanicalContact
    primary_boundary = 'bottom_top'
    secondary_boundary = 'top_bottom'
    primary_subdomain = 'primary_lower'
    secondary_subdomain = 'secondary_lower'
    secondary_variable = disp_z
    component = z
    direction = direction_2
    use_displaced_mesh = true
    compute_lm_residuals = false
    weighted_velocities_uo = friction_uo
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
  petsc_options_iname = '-pc_type -pc_factor_mat_solver_type'
  petsc_options_value = 'lu       superlu_dist'
  l_max_its = 15
  nl_max_its = 90
  nl_rel_tol = 1e-12
  nl_abs_tol = 1e-13
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
[]

[VectorPostprocessors]
[]
