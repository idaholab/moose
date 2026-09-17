[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 2
    ny = 2
  []
  # Assign the right half of the mesh to a second block. The whole mesh is watertight, but the
  # interface between the two blocks is not covered by any sideset/nodeset. Restricting the checks
  # to block 0 should therefore flag the entities on that interface.
  [split]
    type = SubdomainBoundingBoxGenerator
    input = gmg
    block_id = 1
    bottom_left = '0.5 0 0'
    top_right = '1 1 0'
  []
  [diag]
    type = MeshDiagnosticsGenerator
    input = split
    check_for_watertight_sidesets = INFO
    check_for_watertight_nodesets = INFO
    watertight_check_blocks = '0'
  []
[]
