# Two bonded unit squares with non-matching interface meshes. Both are rotated rigidly by th
# about the origin, and the top one is also moved by dn along the rotated reference normal
# n = R N = (-sin(th), cos(th)) and by ds along the rotated reference tangent
# t = R T1 = (-cos(th), -sin(th)), with N = (0, 1) and T1 = (-1, 0) from
# CohesiveZoneModelTools::computeReferenceRotation. The local jump must be (dn, ds) at every
# interface node: jump_n = dn and jump_t = ds.
th = 0.6
dn = 0.01
ds = 0.02

[Mesh]
  [bottom]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 2
    boundary_name_prefix = bottom
  []
  [top]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 3
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
  displacements = 'disp_x disp_y'
[]

[Physics/SolidMechanics/QuasiStatic/all]
  strain = FINITE
  add_variables = true
  use_automatic_differentiation = true
  block = '0 2'
[]

# Rigid motion u = (R - I) X on the bottom block and u = (R - I) X + dn n + ds t on the top block
[Functions]
  [bx]
    type = ParsedFunction
    expression = '(cos(${th})-1)*x - sin(${th})*y'
  []
  [by]
    type = ParsedFunction
    expression = 'sin(${th})*x + (cos(${th})-1)*y'
  []
  [tx]
    type = ParsedFunction
    expression = '(cos(${th})-1)*x - sin(${th})*y - ${dn}*sin(${th}) - ${ds}*cos(${th})'
  []
  [ty]
    type = ParsedFunction
    expression = 'sin(${th})*x + (cos(${th})-1)*y + ${dn}*cos(${th}) - ${ds}*sin(${th})'
  []
[]

[BCs]
  [bx]
    type = FunctionDirichletBC
    variable = disp_x
    boundary = 'bottom_bottom bottom_left bottom_right bottom_top'
    function = bx
  []
  [by]
    type = FunctionDirichletBC
    variable = disp_y
    boundary = 'bottom_bottom bottom_left bottom_right bottom_top'
    function = by
  []
  [tx]
    type = FunctionDirichletBC
    variable = disp_x
    boundary = 'top_bottom top_top top_left top_right'
    function = tx
  []
  [ty]
    type = FunctionDirichletBC
    variable = disp_y
    boundary = 'top_bottom top_top top_left top_right'
    function = ty
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
[]

[AuxVariables]
  [jump_n]
  []
  [jump_t]
  []
[]

[AuxKernels]
  [jump_n]
    type = CohesiveZoneMortarUserObjectAux
    variable = jump_n
    user_object = czm_uo
    cohesive_zone_quantity = local_normal_jump
  []
  [jump_t]
    type = CohesiveZoneMortarUserObjectAux
    variable = jump_t
    user_object = czm_uo
    cohesive_zone_quantity = local_tangential_jump_1
  []
[]

[Postprocessors]
  [jump_n]
    type = ElementAverageValue
    variable = jump_n
    block = 10001
  []
  [jump_t]
    type = ElementAverageValue
    variable = jump_t
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
