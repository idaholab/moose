# Union of a generator-built circle and a non-convex L-shape read from
# l_shape_boundary.msh. A point is contained when inside either geometry.

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
  [lshape]
    type = FileMeshGenerator
    file = l_shape_boundary.msh
    save_with_name = 'lshape'
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
    surface_mesh = lshape
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
