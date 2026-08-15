[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 3
    nx = 4
    ny = 4
    nz = 4
    xmin = -1
    ymin = -1
    zmin = -1
    elem_type = HEX8
  []
  [lsc]
    type = CutMeshByLevelSetGenerator
    input = gmg
    cut_interface = polyhedra
    # Retain the OUTSIDE of the sphere: each cell crossing the sphere has a non-convex
    # retained region (cube minus a spherical dimple), which a single convex C0Polyhedron
    # cannot represent. The generator should detect this and emit a clear error.
    level_set = '0.6 - x*x - y*y - z*z'
    cut_face_name = sph
  []
[]
