# Covers the zero-distance early-return in UnsignedDistanceToSurfaceMesh::gradient
# by querying the function at a point that lies exactly on the surface mesh.

[Problem]
  solve = false
[]

[Mesh]
  [boundary_mesh]
    type = GeneratedMeshGenerator
    dim = 1
    xmin = 0.0
    xmax = 1.0
    nx = 1
    save_with_name = 'boundary_mesh'
  []
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    xmin = -1.0
    xmax = 2.0
    ymin = -1.0
    ymax = 1.0
    nx = 6
    ny = 4
    subdomain_ids = '1'
  []
  add_subdomain_ids = 2
  final_generator = 'gen'
[]

[UserObjects]
  [TreeBuilder]
    type = SBMSurfaceMeshBuilder
    surface_mesh = boundary_mesh
  []
  [distance_to_surface]
    type = ShortestDistanceToSurface
    surfaces = 'dist_edge'
  []
[]

[Functions]
  [dist_edge]
    type = UnsignedDistanceToSurfaceMesh
    builder = TreeBuilder
  []
[]

[AuxVariables]
  # Distance evaluated via the mesh function at a point exactly on the edge.
  # Forces UnsignedDistanceToSurfaceMesh::gradient's dist<=TOLERANCE branch
  # (SBMUtils::distanceVectorFromFunction calls func->gradient internally).
  [dist_on_surface]
    order = CONSTANT
    family = MONOMIAL
  []
[]

[AuxKernels]
  [dist_on_surface]
    type = ShortestDistanceToSurfaceTestAux
    distance_to_surface = distance_to_surface
    method = distance_by_func
    function = dist_edge
    at_point = '0.5 0.0 0.0'  # midpoint of the lone edge: on surface
    variable = dist_on_surface
    execute_on = 'INITIAL'
  []
[]

[Executioner]
  type = Steady
[]

[Outputs]
  exodus = true
[]
