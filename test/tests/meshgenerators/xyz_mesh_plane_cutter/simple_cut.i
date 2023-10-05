[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 3
    nx = 6
    ny = 6
    nz = 6
    zmin = 0
    zmax = 0.6
  []
  [block_1]
    type = ParsedSubdomainMeshGenerator
    input = gmg
    combinatorial_geometry = 'z>0.1'
    block_id = 1
  []
  [block_2]
    type = ParsedSubdomainMeshGenerator
    input = block_1
    combinatorial_geometry = 'z>0.3'
    block_id = 2
  []
  [cut]
    type = XYZMeshPlaneCutter
    input = block_2
    plane_point = '0.5 0.5 0.3'
    plane_normal = '1.0 0.9 0.8'
    cut_face_id = 12345
  []
[]
