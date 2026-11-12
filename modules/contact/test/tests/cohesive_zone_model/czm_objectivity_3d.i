# Two bonded unit cubes with non-matching interface meshes. Both are mapped homogeneously by
# F = R U, where R = [9 -12 20; 20 15 0; -12 16 15] / 25 is the rotation by 2 acos(4/5) about the
# axis (1, 2, 2) / 3, tilted against the interface normal N = (0, 1, 0). The default U = I is a
# rigid rotation; a U that stretches and shears the interface plane (x, z) leaves the normal
# opening unchanged and turns the tangents by the polar rotation of the in-plane surface map. The
# top cube is also offset by F d0 with d0 = (ds1, dn, -ds2), so with T1 = (1, 0, 0) and
# T2 = (0, 0, -1) from CohesiveZoneModelTools::computeReferenceRotation the rigid case gives the
# local jump (dn, ds1, ds2) at every interface node: jump_n = dn, jump_t1 = ds1 and jump_t2 = ds2.
dn = 0.01
ds1 = 0.02
ds2 = 0.03
# Rows of U
u11 = 1
u13 = 0
u31 = 0
u33 = 1
[Mesh]
  [bottom]
    type = GeneratedMeshGenerator
    dim = 3
    boundary_name_prefix = bottom
  []
  [top]
    type = GeneratedMeshGenerator
    dim = 3
    nx = 2
    ymin = 1
    ymax = 2
    boundary_name_prefix = top
    boundary_id_offset = 10
    subdomain_ids = 2
  []
  [combined]
    type = MeshCollectionGenerator
    inputs = 'bottom top'
  []
  [secondary]
    type = LowerDBlockFromSidesetGenerator
    input = combined
    sidesets = bottom_top
    new_block_id = 10001
  []
  [primary]
    type = LowerDBlockFromSidesetGenerator
    input = secondary
    sidesets = top_bottom
    new_block_id = 10000
  []
[]

[GlobalParams]
  displacements = 'disp_x disp_y disp_z'
[]

[Physics/SolidMechanics/QuasiStatic/all]
  strain = FINITE
  add_variables = true
  use_automatic_differentiation = true
  block = '0 2'
[]

# u = F X - X on the bottom cube and u = F (X + d0) - X on the top cube, F = R U
[Functions]
  [bx]
    type = ParsedFunction
    expression = '(9*(${u11}*x + ${u13}*z) - 12*y + 20*(${u31}*x + ${u33}*z)) / 25 - x'
  []
  [by]
    type = ParsedFunction
    expression = '(20*(${u11}*x + ${u13}*z) + 15*y) / 25 - y'
  []
  [bz]
    type = ParsedFunction
    expression = '(-12*(${u11}*x + ${u13}*z) + 16*y + 15*(${u31}*x + ${u33}*z)) / 25 - z'
  []
  [tx]
    type = ParsedFunction
    expression = '(9*(${u11}*(x+${ds1}) + ${u13}*(z-${ds2})) - 12*(y+${dn}) + 20*(${u31}*(x+${ds1}) + ${u33}*(z-${ds2}))) / 25 - x'
  []
  [ty]
    type = ParsedFunction
    expression = '(20*(${u11}*(x+${ds1}) + ${u13}*(z-${ds2})) + 15*(y+${dn})) / 25 - y'
  []
  [tz]
    type = ParsedFunction
    expression = '(-12*(${u11}*(x+${ds1}) + ${u13}*(z-${ds2})) + 16*(y+${dn}) + 15*(${u31}*(x+${ds1}) + ${u33}*(z-${ds2}))) / 25 - z'
  []
[]

[BCs]
  [bx]
    type = FunctionDirichletBC
    variable = disp_x
    boundary = 'bottom_bottom bottom_top'
    function = bx
  []
  [by]
    type = FunctionDirichletBC
    variable = disp_y
    boundary = 'bottom_bottom bottom_top'
    function = by
  []
  [bz]
    type = FunctionDirichletBC
    variable = disp_z
    boundary = 'bottom_bottom bottom_top'
    function = bz
  []
  [tx]
    type = FunctionDirichletBC
    variable = disp_x
    boundary = 'top_bottom top_top'
    function = tx
  []
  [ty]
    type = FunctionDirichletBC
    variable = disp_y
    boundary = 'top_bottom top_top'
    function = ty
  []
  [tz]
    type = FunctionDirichletBC
    variable = disp_z
    boundary = 'top_bottom top_top'
    function = tz
  []
[]

[Materials]
  [strength]
    type = GenericConstantMaterial
    prop_names = 'normal_strength shear_strength'
    prop_values = '50 50'
  []
  [elasticity]
    block = '0 2'
    type = ADComputeIsotropicElasticityTensor
    youngs_modulus = 1e8
    poissons_ratio = 0.3
  []
  [stress]
    block = '0 2'
    type = ADComputeFiniteStrainElasticStress
  []
[]

[UserObjects/czm_uo]
  type = BilinearMixedModeCohesiveZoneModel
  primary_boundary = top_bottom
  secondary_boundary = bottom_top
  primary_subdomain = 10000
  secondary_subdomain = 10001
  disp_x = disp_x
  disp_y = disp_y
  disp_z = disp_z
  penalty_stiffness = 1000
  normal_strength = normal_strength
  shear_strength = shear_strength
  GI_c = 5
  GII_c = 5
  power_law_parameter = 2
[]

[Constraints]
  [cx]
    type = MortarGenericTraction
    primary_boundary = top_bottom
    secondary_boundary = bottom_top
    primary_subdomain = 10000
    secondary_subdomain = 10001
    secondary_variable = disp_x
    component = x
    cohesive_zone_uo = czm_uo
  []
  [cy]
    type = MortarGenericTraction
    primary_boundary = top_bottom
    secondary_boundary = bottom_top
    primary_subdomain = 10000
    secondary_subdomain = 10001
    secondary_variable = disp_y
    component = y
    cohesive_zone_uo = czm_uo
  []
  [cz]
    type = MortarGenericTraction
    primary_boundary = top_bottom
    secondary_boundary = bottom_top
    primary_subdomain = 10000
    secondary_subdomain = 10001
    secondary_variable = disp_z
    component = z
    cohesive_zone_uo = czm_uo
  []
[]

[AuxVariables]
  [jump_n]
  []
  [jump_t1]
  []
  [jump_t2]
  []
[]

[AuxKernels]
  [jump_n]
    type = CohesiveZoneMortarUserObjectAux
    variable = jump_n
    user_object = czm_uo
    cohesive_zone_quantity = local_normal_jump
  []
  [jump_t1]
    type = CohesiveZoneMortarUserObjectAux
    variable = jump_t1
    user_object = czm_uo
    cohesive_zone_quantity = local_tangential_jump_1
  []
  [jump_t2]
    type = CohesiveZoneMortarUserObjectAux
    variable = jump_t2
    user_object = czm_uo
    cohesive_zone_quantity = local_tangential_jump_2
  []
[]

[Postprocessors]
  [jump_n]
    type = ElementAverageValue
    variable = jump_n
    block = 10001
  []
  [jump_t1]
    type = ElementAverageValue
    variable = jump_t1
    block = 10001
  []
  [jump_t2]
    type = ElementAverageValue
    variable = jump_t2
    block = 10001
  []
[]

[Executioner]
  type = Transient
  solve_type = NEWTON
  num_steps = 1
[]

[Outputs]
  csv = true
[]
