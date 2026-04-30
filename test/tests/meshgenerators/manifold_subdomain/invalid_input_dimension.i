[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 4
    ny = 4
  []
  [stl]
    type = FileMeshGenerator
    file = cube_ascii.stl
  []
  [apply]
    type = ManifoldSubdomainGenerator
    input = gmg
    manifold = stl
    block_id = 1
  []
[]
