# Framework point-in-polyhedron test on a closed 2D EDGE2 surface read from a
# gmsh file, using only MooseTestApp objects. The surface is a non-convex L-shaped
# closed loop.

[Problem]
  solve = false
[]

[Mesh]
  [surface_mesh]
    type = FileMeshGenerator
    file = l_shape_boundary.msh
    save_with_name = 'surface_mesh'
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
    user_object = in_out_test
    execute_on = 'initial'
  []
[]

[UserObjects]
  [surface_builder]
    type = BoundaryMeshBuilder
    surface_mesh = surface_mesh
  []

  [in_out_test]
    type = PointInPolyhedronCheckUO
    builder = surface_builder
    point_containment_method = pca_ray
  []
[]

[Executioner]
  type = Steady
[]

[Outputs]
  exodus = true
[]
