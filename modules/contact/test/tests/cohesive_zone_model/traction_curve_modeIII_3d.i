# Purpose: Verify out-of-plane (mode-III) shear traction in a 3D single-element geometry.
# Verifies: cohesive_traction_tangential_2 is nonzero under z-shear; normal traction remains zero.

[Mesh]
  [base]
    type = GeneratedMeshGenerator
    dim = 3
    xmax = 1.1
    xmin = -0.1
    ymax = 1
    zmax = 1
    nx = 1
    ny = 1
    nz = 1
  []
  [rename_base]
    type = RenameBoundaryGenerator
    input = base
    old_boundary = 'top bottom left right front back'
    new_boundary = 'top_base bottom_base left_base right_base front_base back_base'
  []
  [base_id]
    type = SubdomainIDGenerator
    input = rename_base
    subdomain_id = 1
  []

  [top]
    type = GeneratedMeshGenerator
    dim = 3
    xmax = 1
    ymin = 1
    ymax = 2
    zmax = 1
    nx = 1
    ny = 1
    nz = 1
  []
  [rename_top]
    type = RenameBoundaryGenerator
    input = top
    old_boundary = 'top bottom left right front back'
    new_boundary = '100 101 102 103 104 105'
  []
  [top_id]
    type = SubdomainIDGenerator
    input = rename_top
    subdomain_id = 2
  []
  [combined]
    type = MeshCollectionGenerator
    inputs = 'base_id top_id'
  []
  [top_node]
    type = ExtraNodesetGenerator
    coord = '0 2 0'
    input = combined
    new_boundary = top_node
  []
  [bottom_node]
    type = ExtraNodesetGenerator
    coord = '-0.1 0 0'
    input = top_node
    new_boundary = bottom_node
  []

  [secondary]
    type = LowerDBlockFromSidesetGenerator
    new_block_id = 10001
    new_block_name = 'secondary_lower'
    sidesets = 'top_base'
    input = bottom_node
  []
  [primary]
    type = LowerDBlockFromSidesetGenerator
    new_block_id = 10000
    sidesets = '101'
    new_block_name = 'primary_lower'
    input = secondary
  []

  allow_renumbering = false
[]

[GlobalParams]
  displacements = 'disp_x disp_y disp_z'
[]

[Physics]
  [SolidMechanics]
    [QuasiStatic]
      [all]
        strain = FINITE
        add_variables = true
        use_automatic_differentiation = true
        decomposition_method = TaylorExpansion
        block = '1 2'
      []
    []
  []
[]

[AuxVariables]
  [damage]
  []
  [local_normal_jump]
  []
  [cohesive_traction_normal]
  []
  [cohesive_traction_tangential_1]
  []
  [cohesive_traction_tangential_2]
  []
  [cohesive_traction_effective]
  []
  [local_tangential_jump_1]
  []
  [local_tangential_jump_2]
  []
[]

[AuxKernels]
  [cohesive_damage]
    type = CohesiveZoneMortarUserObjectAux
    variable = damage
    user_object = czm_uo
    cohesive_zone_quantity = cohesive_damage
  []
  [local_normal_jump]
    type = CohesiveZoneMortarUserObjectAux
    variable = local_normal_jump
    user_object = czm_uo
    cohesive_zone_quantity = local_normal_jump
  []
  [cohesive_traction_normal]
    type = CohesiveZoneMortarUserObjectAux
    variable = cohesive_traction_normal
    user_object = czm_uo
    cohesive_zone_quantity = cohesive_traction_normal
  []
  [cohesive_traction_tangential_1]
    type = CohesiveZoneMortarUserObjectAux
    variable = cohesive_traction_tangential_1
    user_object = czm_uo
    cohesive_zone_quantity = cohesive_traction_tangential_1
  []
  [cohesive_traction_tangential_2]
    type = CohesiveZoneMortarUserObjectAux
    variable = cohesive_traction_tangential_2
    user_object = czm_uo
    cohesive_zone_quantity = cohesive_traction_tangential_2
  []
  [cohesive_traction_effective]
    type = CohesiveZoneMortarUserObjectAux
    variable = cohesive_traction_effective
    user_object = czm_uo
    cohesive_zone_quantity = cohesive_traction_effective
  []
  [local_tangential_jump_1]
    type = CohesiveZoneMortarUserObjectAux
    variable = local_tangential_jump_1
    user_object = czm_uo
    cohesive_zone_quantity = local_tangential_jump_1
  []
  [local_tangential_jump_2]
    type = CohesiveZoneMortarUserObjectAux
    variable = local_tangential_jump_2
    user_object = czm_uo
    cohesive_zone_quantity = local_tangential_jump_2
  []
[]

[BCs]
  [fix_x]
    type = DirichletBC
    preset = true
    value = 0.0
    boundary = bottom_node
    variable = disp_x
  []
  [fix_bottom_y]
    type = DirichletBC
    boundary = bottom_base
    variable = disp_y
    value = 0
    preset = true
  []
  [fix_bottom_z]
    type = DirichletBC
    boundary = bottom_base
    variable = disp_z
    value = 0
    preset = true
  []
  [fix_top_x]
    type = DirichletBC
    preset = true
    boundary = 100
    variable = disp_x
    value = 0
  []
  [fix_top_y]
    type = DirichletBC
    preset = true
    boundary = 100
    variable = disp_y
    value = 0
  []
  [top_z]
    type = FunctionDirichletBC
    boundary = 100
    variable = disp_z
    function = '0.3*t'
    preset = true
  []
[]

[Materials]
  [normal_strength]
    type = GenericConstantMaterial
    prop_names = 'normal_strength'
    prop_values = '50'
  []
  [shear_strength]
    type = GenericConstantMaterial
    prop_names = 'shear_strength'
    prop_values = '50'
  []
  [stress]
    type = ADComputeFiniteStrainElasticStress
    block = '1 2'
  []
  [elasticity_tensor]
    type = ADComputeElasticityTensor
    fill_method = symmetric_isotropic_E_nu
    C_ijkl = '1e8 0.3'
    block = '1 2'
  []
[]

[UserObjects]
  [czm_uo]
    type = BilinearMixedModeCohesiveZoneModel
    primary_boundary = 101
    secondary_boundary = 'top_base'
    primary_subdomain = 10000
    secondary_subdomain = 10001

    disp_x = disp_x
    disp_y = disp_y
    disp_z = disp_z
    secondary_variable = disp_x
    use_physical_gap = true

    correct_edge_dropping = true
    set_compressive_traction_to_zero = true

    friction_coefficient = 0.0
    penalty = 0.0
    penalty_friction = 0.0

    normal_strength = 'normal_strength'
    shear_strength = 'shear_strength'
    penalty_stiffness = 1000
    power_law_parameter = 2.0
    GI_c = 5
    GII_c = 5
    displacements = 'disp_x disp_y disp_z'
  []
[]

[Constraints]
  [x]
    type = NormalMortarMechanicalContact
    primary_boundary = 101
    secondary_boundary = 'top_base'
    primary_subdomain = 10000
    secondary_subdomain = 10001
    secondary_variable = disp_x
    component = x
    use_displaced_mesh = true
    compute_lm_residuals = false
    weighted_gap_uo = czm_uo
  []
  [y]
    type = NormalMortarMechanicalContact
    primary_boundary = 101
    secondary_boundary = 'top_base'
    primary_subdomain = 10000
    secondary_subdomain = 10001
    secondary_variable = disp_y
    component = y
    use_displaced_mesh = true
    compute_lm_residuals = false
    weighted_gap_uo = czm_uo
  []
  [z]
    type = NormalMortarMechanicalContact
    primary_boundary = 101
    secondary_boundary = 'top_base'
    primary_subdomain = 10000
    secondary_subdomain = 10001
    secondary_variable = disp_z
    component = z
    use_displaced_mesh = true
    compute_lm_residuals = false
    weighted_gap_uo = czm_uo
  []
  [c_x]
    type = MortarGenericTraction
    primary_boundary = 101
    secondary_boundary = 'top_base'
    primary_subdomain = 10000
    secondary_subdomain = 10001
    secondary_variable = disp_x
    component = x
    use_displaced_mesh = true
    compute_lm_residuals = false
    cohesive_zone_uo = czm_uo
  []
  [c_y]
    type = MortarGenericTraction
    primary_boundary = 101
    secondary_boundary = 'top_base'
    primary_subdomain = 10000
    secondary_subdomain = 10001
    secondary_variable = disp_y
    component = y
    use_displaced_mesh = true
    compute_lm_residuals = false
    cohesive_zone_uo = czm_uo
  []
  [c_z]
    type = MortarGenericTraction
    primary_boundary = 101
    secondary_boundary = 'top_base'
    primary_subdomain = 10000
    secondary_subdomain = 10001
    secondary_variable = disp_z
    component = z
    use_displaced_mesh = true
    compute_lm_residuals = false
    cohesive_zone_uo = czm_uo
  []
[]

[Postprocessors]
  [jump_n]
    type = ElementAverageValue
    variable = local_normal_jump
    block = secondary_lower
  []
  [traction_n]
    type = ElementAverageValue
    variable = cohesive_traction_normal
    block = secondary_lower
  []
  [traction_t1]
    type = ElementAverageValue
    variable = cohesive_traction_tangential_1
    block = secondary_lower
  []
  [traction_t2]
    type = ElementAverageValue
    variable = cohesive_traction_tangential_2
    block = secondary_lower
  []
  [traction_eff]
    type = ElementAverageValue
    variable = cohesive_traction_effective
    block = secondary_lower
  []
  [cohesive_damage]
    type = ElementAverageValue
    variable = damage
    block = secondary_lower
  []
  [jump_t]
    type = ElementAverageValue
    variable = local_tangential_jump_2
    block = secondary_lower
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

  solve_type = 'PJFNK'
  line_search = none

  petsc_options_iname = '-pc_type'
  petsc_options_value = 'lu'

  automatic_scaling = true
  compute_scaling_once = false
  off_diagonals_in_auto_scaling = true

  l_max_its = 2
  l_tol = 1e-14
  nl_max_its = 100
  nl_rel_tol = 1e-12
  nl_abs_tol = 1e-11
  start_time = 0.0
  dt = 0.01
  end_time = 1.0
  dtmin = 0.01
[]

[Outputs]
  exodus = false
  csv = true
[]
