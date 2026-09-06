# Mixed-representation union. One geometry is a meshed closed surface (a circle
# handed to a BoundaryMeshBuilder and PointInPolyhedronCheckUO); the other is a
# signed level set (a circle signed-distance function checked by
# PointInSignedFunctionCheckUO). PointInUnionCheckUO reports a point contained when
# it is inside either. This exercises both checker types through one union.

[Problem]
  solve = false
[]

[Mesh]
  [circle1]
    type = ParsedCurveGenerator
    x_formula = '1.0 + 0.8 * cos(t)'
    y_formula = '2.0 + 0.8 * sin(t)'
    section_bounding_t_values = '0 ${fparse 2*pi}'
    nums_segments = '48'
    is_closed_loop = true
    save_with_name = 'circle1'
  []
  [gen]
    type = CartesianMeshGenerator
    dim = 2
    dx = '4'
    dy = '4'
    ix = '16'
    iy = '16'
    subdomain_id = '1'
  []
  final_generator = 'gen'
[]

[Functions]
  # Signed distance to a circle at (3, 2), radius 0.8 (negative inside).
  [circle2_sdf]
    type = ParsedFunction
    expression = 'sqrt((x - 3.0)^2 + (y - 2.0)^2) - 0.8'
  []
[]

[AuxVariables]
  [inside]
    family = MONOMIAL
    order = CONSTANT
  []
[]

[AuxKernels]
  [inside]
    type = SpatialUserObjectAux
    variable = inside
    user_object = union
    execute_on = 'initial'
  []
[]

[UserObjects]
  [builder1]
    type = BoundaryMeshBuilder
    surface_mesh = circle1
  []
  [check_mesh]
    type = PointInPolyhedronCheckUO
    builder = builder1
    point_containment_method = pca_ray
  []
  [check_func]
    type = PointInSignedFunctionCheckUO
    function = circle2_sdf
  []
  [union]
    type = PointInUnionCheckUO
    providers = 'check_mesh check_func'
  []
[]

[Executioner]
  type = Steady
[]

[Outputs]
  exodus = true
[]
