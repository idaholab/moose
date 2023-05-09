[GlobalParams]
  displacements = 'disp_x disp_y disp_z'
  volumetric_locking_correction = true
[]

[Mesh]
  [left_block]
    type = GeneratedMeshGenerator
    dim = 3
    nx = 1
    ny = 1
    nz = 1
    xmin = -1.0
    xmax = 0.0
    ymin = -0.5
    ymax = 0.5
    zmin = -0.5
    zmax = 0.5
    elem_type = HEX8
  []
  [left_block_sidesets]
    type = RenameBoundaryGenerator
    input = left_block
    old_boundary = '0 1 2 3 4 5'
    new_boundary = 'left_bottom left_back left_right left_front left_left left_top'
  []
  [left_block_id]
    type = SubdomainIDGenerator
    input = left_block_sidesets
    subdomain_id = 1
  []

  [right_block]
    type = GeneratedMeshGenerator
    dim = 3
    nx = 2
    ny = 2
    nz = 2
    xmin = 0.0
    xmax = 1.0
    ymin = -0.5
    ymax = 0.5
    zmin = -0.5
    zmax = 0.5
    elem_type = HEX8
  []
  [right_block_sidesets]
    type = RenameBoundaryGenerator
    input = right_block
    old_boundary = '0 1 2 3 4 5'
    # new_boundary = 'right_bottom right_back right_right right_front right_left right_top'
    new_boundary = '100 101 102 103 104 105'
  []
  [right_block_sidesets_rename]
    type = RenameBoundaryGenerator
    input = right_block_sidesets
    old_boundary = '100 101 102 103 104 105'
    new_boundary = 'right_bottom right_back right_right right_front right_left right_top'
  []
  [right_block_id]
    type = SubdomainIDGenerator
    input = right_block_sidesets_rename
    subdomain_id = 2
  []

  [combined_mesh]
    type = MeshCollectionGenerator
    inputs = 'left_block_id right_block_id'
  []

  [left_lower]
    type = LowerDBlockFromSidesetGenerator
    input = combined_mesh
    sidesets = 'left_right'
    new_block_id = '10001'
    new_block_name = 'secondary_lower'
  []
  [right_lower]
    type = LowerDBlockFromSidesetGenerator
    input = left_lower
    sidesets = 'right_left'
    new_block_id = '10000'
    new_block_name = 'primary_lower'
  []
[]

[Variables]
  [normal_lm]
    block = 'secondary_lower'
    use_dual = true
  []
[]

[Modules/TensorMechanics/Master]
  [all]
    strain = FINITE
    incremental = true
    add_variables = true
    block = '1 2'
  []
[]

[Functions]
  [horizontal_movement]
    type = PiecewiseLinear
    x = '0 0.1 4'
    y = '0 0.05 0.05'
  []
  [vertical_movement]
    type = PiecewiseLinear
    x = '0 0.1 4'
    y = '0 0 0.3'
  []
[]

[BCs]
  [push_left_x]
    type = FunctionDirichletBC
    variable = disp_x
    boundary = 'left_left'
    function = horizontal_movement
  []
  [push_left_y]
    type = FunctionDirichletBC
    variable = disp_y
    boundary = 'left_left'
    function = vertical_movement
  []
  [fix_left_z]
    type = DirichletBC
    variable = disp_z
    boundary = 'left_left'
    value = 0.0
  []
  [fix_right_x]
    type = DirichletBC
    variable = disp_x
    boundary = 'right_right'
    value = 0.0
  []
  [fix_right_y]
    type = DirichletBC
    variable = disp_y
    boundary = 'right_right'
    value = 0.0
  []
  [fix_right_z]
    type = DirichletBC
    variable = disp_z
    boundary = 'right_right'
    value = 0.0
  []
[]

[Materials]
  [elasticity_tensor_left]
    type = ComputeIsotropicElasticityTensor
    block = 1
    youngs_modulus = 1.0e6
    poissons_ratio = 0.3
  []
  [stress_left]
    type = ComputeFiniteStrainElasticStress
    block = 1
  []

  [elasticity_tensor_right]
    type = ComputeIsotropicElasticityTensor
    block = 2
    youngs_modulus = 1.0e6
    poissons_ratio = 0.3
  []
  [stress_right]
    type = ComputeFiniteStrainElasticStress
    block = 2
  []
[]

[UserObjects]
  [weighted_gap_uo]
    type = LMWeightedGapUserObject
    primary_boundary = '23'
    secondary_boundary = '11'
    primary_subdomain = 'primary_lower'
    secondary_subdomain = 'secondary_lower'
    lm_variable = normal_lm
    disp_x = disp_x
    disp_y = disp_y
    disp_z = disp_z
  []
[]

[Constraints]
  [normal_lm]
    type = ComputeWeightedGapLMMechanicalContact
    primary_boundary = 'right_left'
    secondary_boundary = 'left_right'
    primary_subdomain = 'primary_lower'
    secondary_subdomain = 'secondary_lower'
    variable = normal_lm
    disp_x = disp_x
    disp_y = disp_y
    disp_z = disp_z
    use_displaced_mesh = true
    correct_edge_dropping = true
    weighted_gap_uo = weighted_gap_uo
  []
  [normal_x]
    type = NormalMortarMechanicalContact
    primary_boundary = 'right_left'
    secondary_boundary = 'left_right'
    primary_subdomain = 'primary_lower'
    secondary_subdomain = 'secondary_lower'
    variable = normal_lm
    secondary_variable = disp_x
    component = x
    use_displaced_mesh = true
    compute_lm_residuals = false
    correct_edge_dropping = true
    weighted_gap_uo = weighted_gap_uo
  []
  [normal_y]
    type = NormalMortarMechanicalContact
    primary_boundary = 'right_left'
    secondary_boundary = 'left_right'
    primary_subdomain = 'primary_lower'
    secondary_subdomain = 'secondary_lower'
    variable = normal_lm
    secondary_variable = disp_y
    component = y
    use_displaced_mesh = true
    compute_lm_residuals = false
    correct_edge_dropping = true
    weighted_gap_uo = weighted_gap_uo
  []
  [normal_z]
    type = NormalMortarMechanicalContact
    primary_boundary = 'right_left'
    secondary_boundary = 'left_right'
    primary_subdomain = 'primary_lower'
    secondary_subdomain = 'secondary_lower'
    variable = normal_lm
    secondary_variable = disp_z
    component = z
    use_displaced_mesh = true
    compute_lm_residuals = false
    correct_edge_dropping = true
    weighted_gap_uo = weighted_gap_uo
  []
[]

[Preconditioning]
  [SMP]
    type = SMP
    full = true
  []
[]

[Executioner]
  type = Transient
  solve_type = 'PJFNK'

  petsc_options_iname = '-pc_type -pc_factor_mat_solver_type -pc_factor_shift_type '
                        '-pc_factor_shift_amount'
  petsc_options_value = 'lu    superlu_dist nonzero 1e-10'

  line_search = 'none'

  dt = 0.1
  dtmin = 0.01
  end_time = 0.4

  l_max_its = 20

  nl_max_its = 20
  nl_rel_tol = 1e-6
  nl_abs_tol = 1e-8
  snesmf_reuse_base = false
[]

[Outputs]
  csv = true
  execute_on = 'FINAL'
[]

[Postprocessors]
  [contact]
    type = ContactDOFSetSize
    variable = normal_lm
    subdomain = 'secondary_lower'
  []
  [normal_lm]
    type = ElementAverageValue
    variable = normal_lm
    block = 'secondary_lower'
  []
  [avg_disp_x]
    type = ElementAverageValue
    variable = disp_x
    block = '1 2'
  []
  [avg_disp_y]
    type = ElementAverageValue
    variable = disp_y
    block = '1 2'
  []
  [max_disp_x]
    type = ElementExtremeValue
    variable = disp_x
    block = '1 2'
  []
  [max_disp_y]
    type = ElementExtremeValue
    variable = disp_y
    block = '1 2'
  []
  [min_disp_x]
    type = ElementExtremeValue
    variable = disp_x
    block = '1 2'
    value_type = min
  []
  [min_disp_y]
    type = ElementExtremeValue
    variable = disp_y
    block = '1 2'
    value_type = min
  []
[]
