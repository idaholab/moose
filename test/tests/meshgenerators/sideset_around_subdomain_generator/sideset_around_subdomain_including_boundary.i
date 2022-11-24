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

  [central_block]
    type = SubdomainBoundingBoxGenerator
    input = gmg
    block_id = 2
    bottom_left = '1 0 0'
    top_right = '3 2 2'
  []

  [central_boundary_xminus]
    type = SideSetsAroundSubdomainGenerator
    input = central_block
    block = 2
    new_boundary = 7
    normal = '-1 0 0'
  []

  [central_boundary_xplus]
    type = SideSetsAroundSubdomainGenerator
    input = central_boundary_xminus
    block = 2
    new_boundary = 8
    normal = '1 0 0'
  []

  [central_boundary_y]
    type = SideSetsAroundSubdomainGenerator
    input = central_boundary_xplus
    block = 2
    new_boundary = 9
    normal = '0 1 0'
  []

  [central_boundary_z]
    type = SideSetsAroundSubdomainGenerator
    input = central_boundary_y
    block = 2
    new_boundary = 10
    normal = '0 0 1'
  []
[]

[Outputs]
  exodus = true
[]
