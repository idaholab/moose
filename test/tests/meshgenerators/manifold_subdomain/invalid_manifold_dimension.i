[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 3
    nx = 4
    ny = 4
    nz = 4
  []
  [manifold]
    type = GeneratedMeshGenerator
    dim = 3
  []

  [apply]
    type = ManifoldSubdomainGenerator
    input = gmg
    manifold = manifold
    block_id = 1
  []
[]
