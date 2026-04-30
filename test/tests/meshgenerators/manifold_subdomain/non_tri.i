[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 3
    nx = 4
    ny = 4
    nz = 4
  []
  [manifold_3D]
    type = GeneratedMeshGenerator
    dim = 3
    xmax = 0.5
    ymax = 0.5
    zmax = 0.5
  []
  [manifold_block]
    type = LowerDBlockFromSidesetGenerator
    input = manifold_3D
    sidesets = 'left right bottom top front back'
    new_block_id = '99'
  []
  [manifold]
    type = BlockToMeshConverterGenerator
    input = manifold_block
    target_blocks = '99'
  []

  [apply]
    type = ManifoldSubdomainGenerator
    input = gmg
    manifold = manifold
    block_id = 1
  []
[]
