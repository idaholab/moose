[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 3
    nx = 4
    ny = 4
    nz = 4
  []
  [manifold]
    type = DistributedRectilinearMeshGenerator
    dim = 2
    nx = 10
    ny = 10
  []

  [apply]
    type = ManifoldSubdomainGenerator
    input = gmg
    manifold = manifold
    block_id = 1
  []
[]
