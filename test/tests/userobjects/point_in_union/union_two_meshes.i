# Boolean union of two closed surface meshes (same representation). Two circles are
# saved as EDGE2 curves and each handed to its own BoundaryMeshBuilder and
# PointInPolyhedronCheckUO; PointInUnionCheckUO reports a point contained when it is
# inside either circle. SpatialUserObjectAux stores 1 inside/on, 0 outside.

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
  [circle2]
    type = ParsedCurveGenerator
    x_formula = '3.0 + 0.8 * cos(t)'
    y_formula = '2.0 + 0.8 * sin(t)'
    section_bounding_t_values = '0 ${fparse 2*pi}'
    nums_segments = '48'
    is_closed_loop = true
    save_with_name = 'circle2'
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
  [builder2]
    type = BoundaryMeshBuilder
    surface_mesh = circle2
  []
  [check1]
    type = PointInPolyhedronCheckUO
    builder = builder1
    point_containment_method = pca_ray
  []
  [check2]
    type = PointInPolyhedronCheckUO
    builder = builder2
    point_containment_method = pca_ray
  []
  [union]
    type = PointInUnionCheckUO
    providers = 'check1 check2'
  []
[]

[Executioner]
  type = Steady
[]

[Outputs]
  exodus = true
[]
