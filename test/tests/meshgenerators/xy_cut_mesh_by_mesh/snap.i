# Same as convex_cut.i but with cutter shifted slightly so its boundary lies
# just inside snap_tol of primary grid lines. Demonstrates the snap pre-pass.
[Mesh]
  [primary]
    type = GeneratedMeshGenerator
    nx = 8
    ny = 8
    dim = 2
    xmin = -1
    ymin = -1
    xmax = 1
    ymax = 1
  []
  [cutter]
    type = GeneratedMeshGenerator
    nx = 2
    ny = 2
    dim = 2
    xmin = -0.252
    ymin = -0.252
    xmax = 0.248
    ymax = 0.248
  []
  [cut]
    type = XYCutMeshByMeshGenerator
    input = primary
    cutter = cutter
    mode = REMOVE_INSIDE
    cutting_type = CUT_ELEM_TRI
    new_boundary_id = 20
    snap_tol = 0.01
  []
[]
