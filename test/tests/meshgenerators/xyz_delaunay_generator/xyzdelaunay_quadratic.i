# Second-order (TET10/TET14) tetrahedralization with XYZDelaunayGenerator.
#
# The outer boundary is converted to second order and THEN deformed by a nonlinear
# transform, so its mid-edge nodes lie off the straight-edge midpoint (i.e. it carries
# genuine curvature). XYZDelaunayGenerator inherits that curvature onto the generated
# tets. A second-order hole is stitched in to exercise second-order stitching.

[Mesh]
  # ---- Outer boundary: curved, second order ----
  [gmg]
    type = GeneratedMeshGenerator
    dim = 3
    nx = 2
    ny = 2
    nz = 2
    elem_type = TET4
  []
  [bdy_second]
    type = ElementOrderConversionGenerator
    input = gmg
    conversion_type = SECOND_ORDER
  []
  [outer_bdy]
    type = ParsedNodeTransformGenerator
    input = bdy_second
    x_function = 'x'
    y_function = 'y'
    z_function = 'z + 0.3 * x * y * z'
  []

  # ---- Hole: second order, smaller interior cube ----
  [hgmg]
    type = GeneratedMeshGenerator
    dim = 3
    nx = 1
    ny = 1
    nz = 1
    elem_type = TET4
  []
  [hole_second]
    type = ElementOrderConversionGenerator
    input = hgmg
    conversion_type = SECOND_ORDER
  []
  [hole]
    type = ParsedNodeTransformGenerator
    input = hole_second
    x_function = '0.35 + 0.3 * x'
    y_function = '0.35 + 0.3 * y'
    z_function = '0.35 + 0.3 * z'
  []
  # Give the stitched hole a distinct subdomain id so it differs from the
  # generated tetrahedra (which are subdomain 0).
  [hole_block]
    type = SubdomainIDGenerator
    input = hole
    subdomain_id = 3
  []

  [triang]
    type = XYZDelaunayGenerator
    boundary = 'outer_bdy'
    holes = 'hole_block'
    stitch_holes = 'true'
    desired_volume = 100000
    tet_element_type = TET10
  []
[]

[Executioner]
  type = Steady
[]

[Problem]
  solve = false
[]

# The element type is what these tests exist to verify, and it is reported here
# rather than inferred from node counts.  Counts cannot be golded on this mesh:
# desired_volume = 100000 leaves the 2x2x2 cube essentially unrefined, so the
# point set is near-cospherical and Netgen's tie-break -- and hence the exact
# tetrahedralization -- depends on the order the boundary nodes reach it, which
# differs between replicated and distributed meshes (libMesh's Netgen interface
# serializes the input mesh before triangulating it).  Element types and the
# per-subdomain volumes are invariant to that retriangulation.
[Reporters]
  [mesh_info]
    type = MeshInfo
    items = 'subdomains'
    subdomain_items = 'elem_types volume'
  []
[]

[Outputs]
  file_base = 'xyzdelaunay_quadratic_tet10_out'
  [json]
    type = JSON
    execute_system_information_on = 'NONE'
  []
[]
