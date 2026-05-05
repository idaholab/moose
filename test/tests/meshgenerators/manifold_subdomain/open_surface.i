[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 3
    nx = 4
    ny = 4
    nz = 4
  []
  [stl]
    type = FileMeshGenerator
    file = open_cube_ascii.stl
  []
  [apply]
    type = ManifoldSubdomainGenerator
    input = gmg
    manifold = stl
    block_id = 1
  []
[]
