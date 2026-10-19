# Two unit squares bonded by a mortar CZM. Both are rotated rigidly by th, and the top one is
# moved by dl along the rotated interface normal, so the local jump must be (dl, 0).
th = 0.6
dl = 0.01

[Mesh]
  [bottom]
    type = GeneratedMeshGenerator
    dim = 2
    boundary_name_prefix = bottom
  []
  [top]
    type = GeneratedMeshGenerator
    dim = 2
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
    expression = '(cos(${th})-1)*x - sin(${th})*y - sin(${th})*${dl}'
  []
  [ty]
    type = ParsedFunction
    expression = 'sin(${th})*x + (cos(${th})-1)*y + cos(${th})*${dl}'
  []
[]

[BCs]
  [bx]
    type = FunctionDirichletBC
    variable = disp_x
    boundary = 'bottom_bottom bottom_left bottom_right'
    function = bx
  []
  [by]
    type = FunctionDirichletBC
    variable = disp_y
    boundary = 'bottom_bottom bottom_left bottom_right'
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

[UserObjects/czm]
  type = BilinearMixedModeCohesiveZoneModel
  primary_boundary = top_bottom
  secondary_boundary = bottom_top
  primary_subdomain = 10000
  secondary_subdomain = 10001
  disp_x = disp_x
  disp_y = disp_y
  displacements = 'disp_x disp_y'
  secondary_variable = disp_x
  penalty = 0
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
    cohesive_zone_uo = czm
    use_displaced_mesh = true
  []
  [cy]
    type = MortarGenericTraction
    primary_boundary = top_bottom
    secondary_boundary = bottom_top
    primary_subdomain = 10000
    secondary_subdomain = 10001
    secondary_variable = disp_y
    component = y
    cohesive_zone_uo = czm
    use_displaced_mesh = true
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
    user_object = czm
    cohesive_zone_quantity = local_normal_jump
  []
  [jump_t]
    type = CohesiveZoneMortarUserObjectAux
    variable = jump_t
    user_object = czm
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
