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
  [block_1]
    type = ParsedSubdomainMeshGenerator
    input = gmg
    combinatorial_geometry = 'z>0'
    block_id = 1
  []
  [lsc]
    type = CutMeshByLevelSetGenerator
    input = block_1
    level_set = 'x*x+y*y+z*z-0.81'
    cut_face_id = 345
    cut_face_name =ls
  []
[]
