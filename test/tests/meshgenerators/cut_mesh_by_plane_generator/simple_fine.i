[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 3
    nx = 4
    ny = 4
    nz = 4
    zmin = 0
    zmax = 0.6
  []
  [block_1]
    type = ParsedSubdomainMeshGenerator
    input = gmg
    combinatorial_geometry = 'z>0.3'
    block_id = 1
  []
  [rename]
    type = RenameBlockGenerator
    input = block_1
    old_block = '0 1'
    new_block = 'lower upper'
  []
  [cut]
    type = CutMeshByPlaneGenerator
    input = rename
    plane_point = '0.5 0.5 0.3'
    plane_normal = '1.0 0.9 0.8'
    generate_transition_layer = true
  []
[]
