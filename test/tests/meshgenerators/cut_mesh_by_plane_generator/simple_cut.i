[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 3
    nx = 2
    ny = 2
    nz = 2
    zmin = 0
    zmax = 0.6
  []
  [block_1]
    type = ParsedSubdomainMeshGenerator
    input = gmg
    combinatorial_geometry = 'z>0.3'
    block_id = 1
  []
  [cut]
    type = CutMeshByPlaneGenerator
    input = block_1
    plane_point = '0.5 0.5 0.3'
    plane_normal = '1.0 0.9 0.8'
  []
[]
