# A HEX8 (block 0) sharing a face with a tetrahedralized box (block 1). The shared face is a
# single quad on the hex side but two triangles on the tet side: a non-conforming interface.
# All four corner nodes are shared, so there is NO hanging node and 'examine_non_conformality'
# reports 0; 'examine_nonconforming_faces' detects it (the quad face and the two triangle faces,
# 3 faces total).
[Mesh]
  [hexbox]
    type = GeneratedMeshGenerator
    dim = 3
    nx = 1
    ny = 1
    nz = 1
    xmin = 0
    xmax = 1
  []
  [tetbox0]
    type = GeneratedMeshGenerator
    dim = 3
    nx = 1
    ny = 1
    nz = 1
    xmin = 1
    xmax = 2
  []
  [tetbox_blk]
    type = SubdomainBoundingBoxGenerator
    input = tetbox0
    block_id = 1
    bottom_left = '0.9 -0.1 -0.1'
    top_right = '2.1 1.1 1.1'
  []
  [tetbox]
    type = ElementsToTetrahedronsConverter
    input = tetbox_blk
  []
  [stitch]
    type = StitchMeshGenerator
    inputs = 'hexbox tetbox'
    stitch_boundaries_pairs = 'right left'
    clear_stitched_boundary_ids = true
  []
  [diag]
    type = MeshDiagnosticsGenerator
    input = stitch
    examine_nonconforming_faces = INFO
    examine_non_conformality = INFO
  []
[]
