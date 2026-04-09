[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 3
    nx = 4
    ny = 4
    nz = 4
  []

  [stl]
    type = STLSubdomainGenerator
    input = gmg
    stl_file = open_cube_ascii.stl
    translation = '0.5 0.5 0.5'
    block_id = 1
  []
[]
