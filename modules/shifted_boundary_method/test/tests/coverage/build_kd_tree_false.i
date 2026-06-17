# Covers the build_kd_tree = false branch in SBMSurfaceMeshBuilder::initialSetup.
# Skips constructing the KDTree (no downstream UnsignedDistanceToSurfaceMesh).

[Problem]
  solve = false
[]

[Mesh]
  [boundary_mesh]
    type = FileMeshGenerator
    file = '../distance_calc/star.msh'
  []
  [shift_boundary_mesh]
    type = TransformGenerator
    transform = TRANSLATE
    vector_value = '2.0 2.0 0.0'
    input = boundary_mesh
    save_with_name = 'shift_boundary_mesh'
  []
  [gen]
    type = CartesianMeshGenerator
    dim = 2
    dx = '4'
    dy = '4'
    ix = '8'
    iy = '8'
    subdomain_id = '1'
  []
  add_subdomain_ids = 2
  add_sideset_names = 'SBMinterface'
  final_generator = 'gen'
[]

[UserObjects]
  [TreeBuilder]
    type = SBMSurfaceMeshBuilder
    surface_mesh = shift_boundary_mesh
    build_kd_tree = false
  []
[]

[Executioner]
  type = Steady
[]
