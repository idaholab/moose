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
    type = SideSetsBetweenSubdomainsGenerator
    input = central_block
    primary_block = 2
    paired_block = 0
    new_boundary = 7
  []
[]

[Outputs]
  exodus = true
[]
