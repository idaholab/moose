# Minimal input exercising OrientedBoundingBox::writeMesh / writeRayAlongShortestAxis.
# An anisotropic ellipse gives PCA a well-separated minor axis, so the oriented
# bounding box and the ray along the shortest axis are reproducible. The in-out
# user object builds the OBB and writes the OBB (QUAD4) and ray (EDGE2) meshes
# to file when it is constructed, so no solve, mesh modifier, or variable is
# needed to trigger the write.
a = 1.6
b = 1.0
cx = 2.03
cy = 1.97
n_seg = 48

[Problem]
  solve = false
[]

[Mesh]
  [shift_boundary_mesh]
    type = ParsedCurveGenerator
    x_formula = '${cx} + ${a} * cos(t)'
    y_formula = '${cy} + ${b} * sin(t)'
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
    ix = '16'
    iy = '16'
    subdomain_id = '1'
  []

  final_generator = 'gen'
[]

[UserObjects]
  [TreeBuilder]
    type = SBMSurfaceMeshBuilder
    surface_mesh = shift_boundary_mesh
  []

  [inout_test]
    type = PointInPolyhedronCheckUO
    brute_force = true
    builder = TreeBuilder
    obb_file_name = 'obb_bounds.e'
    ray_file_name = 'ray_bounds.e'
  []
[]

[Executioner]
  type = Steady
[]
