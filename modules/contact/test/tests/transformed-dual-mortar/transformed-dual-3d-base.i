# Shared setup for the 3D uniform-compression mortar-contact patch test on quadratic
# secondary faces, included by transformed-dual-vcp-3d.i; the mesh, variables, physics,
# contact, and postprocessors live here and the wrapper adds the preconditioner and
# executioner.
#
# Two elastic blocks share a node-conforming mortar interface (BreakMeshByBlockGenerator
# splits a single grid at x = 0). The left block is pushed uniformly in +x against the fixed
# right block, giving a spatially constant contact pressure that the transformed dual basis
# (applied automatically for dual mortar on a second-order LM) reproduces exactly -- max,
# min, and average of normal_lm agree.
#
# The mesh is promoted with ElementOrderConversionGenerator using SECOND_ORDER_NONFULL
# (all_second_order(false)): HEX8 -> HEX20 (QUAD8 faces), TET4 -> TET10 (TRI6 faces). Do
# NOT use full promotion: hexes then become HEX27 (QUAD9 faces), which is identity under
# the transform.

[GlobalParams]
  displacements = 'disp_x disp_y disp_z'
[]

[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 3
    # Two cells along x with the cell boundary at x = 0 gives a single conforming
    # interface once the mesh is split into the two blocks below.
    nx = 2
    ny = 1
    nz = 1
    xmin = -1.0
    xmax = 1.0
    ymin = -0.5
    ymax = 0.5
    zmin = -0.5
    zmax = 0.5
    elem_type = HEX8
  []
  [block1]
    type = SubdomainBoundingBoxGenerator
    input = gen
    bottom_left = '-1.0 -0.5 -0.5'
    top_right = '0.0 0.5 0.5'
    block_id = 1
  []
  [block2]
    type = SubdomainBoundingBoxGenerator
    input = block1
    bottom_left = '0.0 -0.5 -0.5'
    top_right = '1.0 0.5 0.5'
    block_id = 2
  []
  [break]
    type = BreakMeshByBlockGenerator
    input = block2
    # Duplicates the nodes on the block 1 / block 2 boundary, creating the two
    # coincident sidesets Block1_Block2 (secondary, on block 1) and
    # Block2_Block1 (primary, on block 2).
    block_pairs = '1 2'
  []
  [to_second_order]
    type = ElementOrderConversionGenerator
    input = break
    conversion_type = SECOND_ORDER_NONFULL
  []
  [secondary_lower]
    type = LowerDBlockFromSidesetGenerator
    input = to_second_order
    sidesets = 'Block1_Block2'
    new_block_id = '10001'
    new_block_name = 'secondary_lower'
  []
  [primary_lower]
    type = LowerDBlockFromSidesetGenerator
    input = secondary_lower
    sidesets = 'Block2_Block1'
    new_block_id = '10000'
    new_block_name = 'primary_lower'
  []
[]

[Variables]
  # Displacements are declared explicitly at second order so the Physics action
  # picks them up (it is intentionally invoked without add_variables here).
  [disp_x]
    order = SECOND
    block = '1 2'
  []
  [disp_y]
    order = SECOND
    block = '1 2'
  []
  [disp_z]
    order = SECOND
    block = '1 2'
  []
  [normal_lm]
    block = 'secondary_lower'
    order = SECOND
    family = LAGRANGE
    use_dual = true
  []
[]

[Physics/SolidMechanics/QuasiStatic]
  [all]
    strain = FINITE
    incremental = true
    block = '1 2'
  []
[]

[Functions]
  [horizontal_movement]
    type = PiecewiseLinear
    x = '0 1'
    y = '0 0.02'
  []
[]

[BCs]
  # Uniaxial compression along x. With poissons_ratio = 0 the exact response has
  # zero lateral strain, so fixing the lateral displacements on the loaded and
  # fixed faces is consistent with the uniform field and the recovered pressure
  # is spatially constant.
  [push_left_x]
    type = FunctionDirichletBC
    variable = disp_x
    boundary = 'left'
    function = horizontal_movement
  []
  [fix_left_y]
    type = DirichletBC
    variable = disp_y
    boundary = 'left'
    value = 0.0
  []
  [fix_left_z]
    type = DirichletBC
    variable = disp_z
    boundary = 'left'
    value = 0.0
  []
  [fix_right_x]
    type = DirichletBC
    variable = disp_x
    boundary = 'right'
    value = 0.0
  []
  [fix_right_y]
    type = DirichletBC
    variable = disp_y
    boundary = 'right'
    value = 0.0
  []
  [fix_right_z]
    type = DirichletBC
    variable = disp_z
    boundary = 'right'
    value = 0.0
  []
[]

[Materials]
  # poissons_ratio = 0 is a deliberate patch-test construction (uniaxial
  # compression -> zero lateral strain -> spatially constant contact pressure),
  # not a physical material value; with nu != 0 the lateral constraints above
  # would fight the Poisson bulge and the pressure would not be constant.
  [elasticity_tensor]
    type = ComputeIsotropicElasticityTensor
    block = '1 2'
    youngs_modulus = 1.0e6
    poissons_ratio = 0.0
  []
  [stress]
    type = ComputeFiniteStrainElasticStress
    block = '1 2'
  []
[]

[UserObjects]
  [weighted_gap_uo]
    type = LMWeightedGapUserObject
    primary_boundary = 'Block2_Block1'
    secondary_boundary = 'Block1_Block2'
    primary_subdomain = 'primary_lower'
    secondary_subdomain = 'secondary_lower'
    correct_edge_dropping = true
    lm_variable = normal_lm
    disp_x = disp_x
    disp_y = disp_y
    disp_z = disp_z
  []
[]

[Constraints]
  [normal_lm]
    type = ComputeWeightedGapLMMechanicalContact
    primary_boundary = 'Block2_Block1'
    secondary_boundary = 'Block1_Block2'
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
    primary_boundary = 'Block2_Block1'
    secondary_boundary = 'Block1_Block2'
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
    primary_boundary = 'Block2_Block1'
    secondary_boundary = 'Block1_Block2'
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
    primary_boundary = 'Block2_Block1'
    secondary_boundary = 'Block1_Block2'
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
  # For the patch test the contact pressure is spatially constant, so the
  # average, maximum, and minimum of normal_lm over the interface must agree.
  [normal_lm]
    type = ElementAverageValue
    variable = normal_lm
    block = 'secondary_lower'
  []
  [max_normal_lm]
    type = ElementExtremeValue
    variable = normal_lm
    block = 'secondary_lower'
  []
  [min_normal_lm]
    type = ElementExtremeValue
    variable = normal_lm
    block = 'secondary_lower'
    value_type = min
  []
[]
