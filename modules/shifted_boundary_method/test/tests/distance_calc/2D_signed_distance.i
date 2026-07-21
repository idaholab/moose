nx = 32
[Problem]
  solve = false
[]

[Mesh]
  [boundary_mesh]
    type = FileMeshGenerator
    file = 'square_boundary.msh'
  []

  [shift_boundary_mesh]
    type = TransformGenerator
    transform = TRANSLATE
    vector_value = '2.0 2.0 0.0' # translation in x, y, z directions
    input = boundary_mesh
    save_with_name = 'shift_boundary_mesh'
  []

  [gen]
    type = CartesianMeshGenerator
    dim = 2
    dx = '4'
    dy = '4'
    ix = '${nx}'
    iy = '${nx}'
    subdomain_id = '1'
  []

  add_subdomain_ids = 2 # outside block
  add_sideset_names = 'SBMinterface'
  final_generator = 'gen'
[]

[UserObjects]
  [TreeBuilder]
    type = SBMSurfaceMeshBuilder
    surface_mesh = shift_boundary_mesh
  []
  [signed_distance_to_surface]
    type = ShortestDistanceToSurface
    surfaces = 'sign_dist_square'
    signed_distance = true
  []
  [distance_to_surface]
    type = ShortestDistanceToSurface
    surfaces = 'dist_square'
  []
  [InOutTest]
    type = PointInPolyhedronCheckUO
    builder = TreeBuilder
  []
[]

[Functions]
  [dist_square]
    type = UnsignedDistanceToSurfaceMesh
    builder = TreeBuilder
  []
  [sign_dist_square]
    type = SignedDistanceToSurfaceMesh
    builder = TreeBuilder
    in_out_test = InOutTest
  []
[]

[Variables]
  [u]
    initial_condition = 1
    block = 1
  []
[]

[AuxVariables]
  [signed_distance]
    order = CONSTANT
    family = MONOMIAL
  []
  [unsigned_distance]
    order = CONSTANT
    family = MONOMIAL
  []
[]

[AuxKernels]
  [signed_dist]
    type = ElementCentroidToSurfaceDistanceAux
    distance_to_surface = signed_distance_to_surface
    variable = signed_distance
    execute_on = 'INITIAL timestep_begin'
  []
  [unsigned_dist]
    type = ElementCentroidToSurfaceDistanceAux
    distance_to_surface = distance_to_surface
    variable = unsigned_distance
    execute_on = 'INITIAL timestep_begin'
  []
[]

[Executioner]
  type = Steady
[]

[Outputs]
  exodus = true
[]
