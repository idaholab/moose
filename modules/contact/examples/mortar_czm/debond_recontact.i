# Purpose: Verify that a fully debonded interface (omega=1) re-establishes compressive contact on closure.
# Verifies: damage stays at 1 after full decohesion; compressive traction is nonzero on recontact via penalty contact.

[Mesh]
  [base]
    type = GeneratedMeshGenerator
    dim = 2
    xmax = 1.1
    ymax = 1
    xmin = -0.1
    nx = 1
    ny = 1
  []
  [rename_base]
    type = RenameBoundaryGenerator
    input = base
    old_boundary = 'top bottom left right'
    new_boundary = 'top_base bottom_base left_base right_base'
  []
  [base_id]
    type = SubdomainIDGenerator
    input = rename_base
    subdomain_id = 1
  []

  [top]
    type = GeneratedMeshGenerator
    dim = 2
    xmax = 1
    ymin = 1
    ymax = 2
    nx = 1
    ny = 1
  []
  [rename_top]
    type = RenameBoundaryGenerator
    input = top
    old_boundary = 'top bottom left right'
    new_boundary = '100 101 102 103'
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
  displacements = 'disp_x disp_y'
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
[]

[BCs]
  [anchor_bottom_x]
    type = DirichletBC
    preset = true
    value = 0.0
    boundary = bottom_node
    variable = disp_x
  []
  [anchor_top_x]
    type = DirichletBC
    preset = true
    boundary = top_node
    variable = disp_x
    value = 0
  []
  # Open past delta_f (=0.2mm at K=1000, N=50, GI_c=5) then reverse into compression.
  # delta_f = 2*GI_c/N = 0.2; loading reaches 0.24 (full debond) then reverses to -0.096.
  [top]
    type = FunctionDirichletBC
    boundary = 100
    variable = disp_y
    function = 'if(t<=0.8, 0.3*t, 0.24 - 0.48*(t-0.8))'
    preset = true
  []
  [roller_bottom_y]
    type = DirichletBC
    boundary = bottom_base
    variable = disp_y
    value = 0
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
    secondary_variable = disp_x
    use_physical_gap = true

    correct_edge_dropping = true
    set_compressive_traction_to_zero = true

    friction_coefficient = 0.0
    penalty = 1e4
    penalty_friction = 0.0

    normal_strength = 'normal_strength'
    shear_strength = 'shear_strength'
    penalty_stiffness = 1000
    power_law_parameter = 2.0
    GI_c = 5
    GII_c = 5
    displacements = 'disp_x disp_y'
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
  [cohesive_damage]
    type = ElementAverageValue
    variable = damage
    block = secondary_lower
  []
  # Total interface traction (CZM + penalty contact) from bulk stress equilibrium.
  # After full debond (omega=1), CZM traction_n = 0, but this captures penalty contact compression.
  [total_traction_y]
    type = ADSidesetReaction
    stress_tensor = stress
    boundary = top_base
    direction = '0 1 0'
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
  dt = 0.02
  end_time = 1.36
  dtmin = 0.02
[]

[Outputs]
  exodus = false
  csv = true
[]
