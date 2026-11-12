# Two bonded quarter rings meeting on the circle r = 2. The inner ring face has 3 elements and
# the outer ring face 4, so the two faceted interface curves differ: paired mortar points
# are up to |X_primary - X_secondary| ~ 0.034 apart in the reference configuration. Both rings
# move rigidly by a rotation th about the origin and a translation (cx, cy). A rigid motion
# opens no gap, so the local jump must vanish at every interface node: jump_n = jump_t = 0.
# Without the reference-gap correction the rotated u_primary - u_secondary would be ~0.022.
th = 0.7
cx = 0.3
cy = -0.2

[Mesh]
  [inner]
    type = AnnularMeshGenerator
    rmin = 1
    rmax = 2
    dmin = 0
    dmax = 90
    nt = 3
    boundary_name_prefix = inner
  []
  [outer]
    type = AnnularMeshGenerator
    rmin = 2
    rmax = 3
    dmin = 0
    dmax = 90
    nt = 4
    quad_subdomain_id = 2
    boundary_name_prefix = outer
    boundary_id_offset = 10
  []
  [combined]
    type = MeshCollectionGenerator
    inputs = 'inner outer'
  []
  [secondary]
    type = LowerDBlockFromSidesetGenerator
    input = combined
    sidesets = inner_rmax
    new_block_id = 10001
  []
  [primary]
    type = LowerDBlockFromSidesetGenerator
    input = secondary
    sidesets = outer_rmin
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

# Rigid motion u = (R - I) X + (cx, cy) on both rings
[Functions]
  [ux]
    type = ParsedFunction
    expression = '(cos(${th})-1)*x - sin(${th})*y + ${cx}'
  []
  [uy]
    type = ParsedFunction
    expression = 'sin(${th})*x + (cos(${th})-1)*y + ${cy}'
  []
[]

[BCs]
  [ux]
    type = FunctionDirichletBC
    variable = disp_x
    boundary = 'inner_rmin inner_rmax inner_dmin inner_dmax outer_rmin outer_rmax outer_dmin '
               'outer_dmax'
    function = ux
  []
  [uy]
    type = FunctionDirichletBC
    variable = disp_y
    boundary = 'inner_rmin inner_rmax inner_dmin inner_dmax outer_rmin outer_rmax outer_dmin '
               'outer_dmax'
    function = uy
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
  primary_boundary = outer_rmin
  secondary_boundary = inner_rmax
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
    primary_boundary = outer_rmin
    secondary_boundary = inner_rmax
    primary_subdomain = 10000
    secondary_subdomain = 10001
    secondary_variable = disp_x
    component = x
    cohesive_zone_uo = czm_uo
  []
  [cy]
    type = MortarGenericTraction
    primary_boundary = outer_rmin
    secondary_boundary = inner_rmax
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
    type = NodalExtremeValue
    variable = jump_n
    value_type = max_abs
    block = 10001
  []
  [jump_t]
    type = NodalExtremeValue
    variable = jump_t
    value_type = max_abs
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
