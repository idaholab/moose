nx = 16
# Asymmetric immersed geometry: a non-circular closed curve. A circle is maximally
# symmetric (every diameter is a symmetry axis), so an element centroid can sit
# exactly on a symmetry axis and become equidistant to two surface segments. That
# KDTree nearest-segment tie is broken only by floating-point round-off, which
# differs across platforms and flips the gradient direction. Deforming the boundary
# with a radial modulation removes all symmetry, so every centroid has a unique
# nearest segment and the in-out sign / gradient stay reproducible across platforms.
R = 1.3
cx = 2.03
cy = 1.97
# Control the boundary asymmetry (radial modulation amplitudes).
a = 0.18
b = 0.10
n_seg = 64
[Problem]
  solve = false
[]

[Mesh]
  [shift_boundary_mesh]
    type = ParsedCurveGenerator
    x_formula = '${cx} + ${R} * (1 + ${a} * cos(t) + ${b} * sin(2*t)) * cos(t)'
    y_formula = '${cy} + ${R} * (1 + ${a} * cos(t) + ${b} * sin(2*t)) * sin(t)'
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
    # A user-selected axis-aligned ray avoids the PCA/SVD direction ambiguity of
    # an isotropic shape (circle), which is not reproducible across platforms.
    point_containment_method = user_selected_ray
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
  # Vector field capturing the signed-distance gradient direction. Sampling the
  # gradient at every element centroid exercises points both inside and outside
  # the circle, so the gold pins down the outward-pointing direction on both sides.
  [signed_distance_grad]
    order = CONSTANT
    family = MONOMIAL_VEC
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
  [signed_dist_grad]
    type = FunctorElementalGradientAux
    functor = sign_dist_square
    variable = signed_distance_grad
    execute_on = 'INITIAL timestep_begin'
  []
[]

[Executioner]
  type = Steady
[]

[Outputs]
  exodus = true
[]
