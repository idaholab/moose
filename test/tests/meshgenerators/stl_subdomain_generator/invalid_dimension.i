[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 4
    ny = 4
  []

  [stl]
    type = STLSubdomainGenerator
    input = gmg
    stl_file = cube_ascii.stl
    translation = '0.5 0.5 0.0'
    block_id = 1
  []
[]
