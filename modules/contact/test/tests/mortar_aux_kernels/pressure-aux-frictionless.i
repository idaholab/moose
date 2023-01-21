[GlobalParams]
  displacements = 'disp_x disp_y'
  volumetric_locking_correction = true
[]

[Mesh]
  [left_block]
    type = GeneratedMeshGenerator
    dim = 2
    xmin = -0.35
    xmax = -0.05
    ymin = -1
    ymax = 0
    nx = 1
    ny = 3
    elem_type = QUAD4
  []
  [left_block_sidesets]
    type = RenameBoundaryGenerator
    input = left_block
    old_boundary = '0 1 2 3'
    new_boundary = '10 11 12 13'
  []
  [left_block_sideset_names]
    type = RenameBoundaryGenerator
    input = left_block_sidesets
    old_boundary = '10 11 12 13'
    new_boundary = 'l_bottom l_right l_top l_left'
  []
  [left_block_id]
    type = SubdomainIDGenerator
    input = left_block_sideset_names
    subdomain_id = 1
  []
  [right_block]
    type = GeneratedMeshGenerator
    dim = 2
    xmin = 0
    xmax = 0.3
    ymin = -1
    ymax = 0
    nx = 1
    ny = 2
    elem_type = QUAD4
  []
  [right_block_sidesets]
    type = RenameBoundaryGenerator
    input = right_block
    old_boundary = '0 1 2 3'
    new_boundary = '20 21 22 23'
  []
  [right_block_sideset_names]
    type = RenameBoundaryGenerator
    input = right_block_sidesets
    old_boundary = '20 21 22 23'
    new_boundary = 'r_bottom r_right r_top r_left'
  []
  [right_block_id]
    type = SubdomainIDGenerator
    input = right_block_sideset_names
    subdomain_id = 2
  []

  [combined_mesh]
    type = MeshCollectionGenerator
    inputs = 'left_block_id right_block_id'
  []

  [left_lower]
    type = LowerDBlockFromSidesetGenerator
    input = combined_mesh
    sidesets = '11'
    new_block_id = '10001'
    new_block_name = 'secondary_lower'
  []
  [right_lower]
    type = LowerDBlockFromSidesetGenerator
    input = left_lower
    sidesets = '23'
    new_block_id = '10000'
    new_block_name = 'primary_lower'
  []

  uniform_refine = 1
[]

[Variables]
  [lm_x]
    block = 'secondary_lower'
    use_dual = true
  []
  [lm_y]
    block = 'secondary_lower'
    use_dual = true
  []
[]

[AuxVariables]
  [normal_lm]
    family = LAGRANGE
    order = FIRST
  []
[]

[AuxKernels]
  [normal_lm]
    type = MortarPressureComponentAux
    variable = normal_lm
    primary_boundary = '23'
    secondary_boundary = '11'
    lm_var_x = lm_x
    lm_var_y = lm_y
    component = 'NORMAL'
    boundary = '11'
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
    type = ParsedFunction
    expression = '0.1 * t'
  []
  [vertical_movement]
    type = ConstantFunction
    value = '0.0'
  []
[]

[BCs]
  [push_left_x]
    type = FunctionDirichletBC
    variable = disp_x
    boundary = 13
    function = horizontal_movement
  []
  [fix_right_x]
    type = DirichletBC
    variable = disp_x
    boundary = 21
    value = 0.0
  []
  [fix_right_y]
    type = DirichletBC
    variable = disp_y
    boundary = 21
    value = 0.0
  []
  [push_left_y]
    type = FunctionDirichletBC
    variable = disp_y
    boundary = 13
    function = vertical_movement
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

[Constraints]
  [weighted_gap_lm]
    type = ComputeWeightedGapCartesianLMMechanicalContact
    primary_boundary = '23'
    secondary_boundary = '11'
    primary_subdomain = 'primary_lower'
    secondary_subdomain = 'secondary_lower'
    lm_x = lm_x
    lm_y = lm_y
    variable = lm_x # This can be anything really
    disp_x = disp_x
    disp_y = disp_y
    use_displaced_mesh = true
    correct_edge_dropping = true
    interpolate_normals = false
  []
  [normal_x]
    type = CartesianMortarMechanicalContact
    primary_boundary = '23'
    secondary_boundary = '11'
    primary_subdomain = 'primary_lower'
    secondary_subdomain = 'secondary_lower'
    variable = lm_x
    secondary_variable = disp_x
    component = x
    use_displaced_mesh = true
    compute_lm_residuals = false
    correct_edge_dropping = true
  []
  [normal_y]
    type = CartesianMortarMechanicalContact
    primary_boundary = '23'
    secondary_boundary = '11'
    primary_subdomain = 'primary_lower'
    secondary_subdomain = 'secondary_lower'
    variable = lm_y
    secondary_variable = disp_y
    component = y
    use_displaced_mesh = true
    compute_lm_residuals = false
    correct_edge_dropping = true
  []
[]

[Preconditioning]
  [smp]
    type = SMP
    full = true
  []
[]

[Executioner]
  type = Transient
  solve_type = 'NEWTON'

  petsc_options_iname = '-pc_type -pc_factor_mat_solver_package -mat_mffd_err -pc_factor_shift_type '
                        '-pc_factor_shift_amount'
  petsc_options_value = 'lu superlu_dist 1e-5          NONZERO               1e-10'

  line_search = none

  dt = 0.1
  dtmin = 0.1
  end_time = 1.0

  l_max_its = 100

  nl_max_its = 20
  nl_rel_tol = 1e-6
  snesmf_reuse_base = false
[]

[Outputs]
  exodus = false
  csv = true
  execute_on = 'FINAL'
[]

[VectorPostprocessors]
  [normal_lm]
    type = NodalValueSampler
    block = 'secondary_lower'
    variable = normal_lm
    sort_by = 'id'
  []
[]
