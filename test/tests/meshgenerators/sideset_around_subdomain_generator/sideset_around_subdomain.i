[Mesh]
  [./gmg]
    type = GeneratedMeshGenerator
    dim = 3
    xmax = 3
    ymax = 3
    zmax = 3
    nx = 3
    ny = 3
    nz = 3
  []

  [./central_block]
    type = SubdomainBoundingBoxGenerator
    input = gmg
    block_id = 2
    bottom_left = '1 1 1'
    top_right = '2 2 2'
  []

  [./central_boundary]
    type = SideSetsAroundSubdomainGenerator
    input = central_block
    block = 2
    new_boundary = 7
  []
[]

[Outputs]
  exodus = true
[]
