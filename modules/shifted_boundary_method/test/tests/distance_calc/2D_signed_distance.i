nx = 16
# Immersed circle geometry: off-grid center so no element centroid lands exactly
# on the surface, keeping the in-out sign reproducible across platforms.
R = 1.3
cx = 2.03
cy = 1.97
n_seg = 48
[Problem]
  solve = false
[]

[Mesh]
  [shift_boundary_mesh]
    type = ParsedCurveGenerator
    x_formula = '${cx} + ${R} * cos(t)'
    y_formula = '${cy} + ${R} * sin(t)'
    section_bounding_t_values = '0 ${fparse 2*pi}'
    nums_segments = '${n_seg}'
    is_closed_loop = true
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
    # Fixed axis-aligned ray avoids the PCA/SVD direction ambiguity of an
    # isotropic shape (circle), which is not reproducible across platforms.
    ray_direction = '1 0 0'
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
