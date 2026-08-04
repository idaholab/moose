[Mesh]
  [cmg]
    type = CartesianMeshGenerator
    dim = 2
    dx = '1 1'
    dy = '1'
    subdomain_id = '0 1'
  []

  [sideset]
    type = SideSetsFromBoundingBoxGenerator
    input = cmg
    included_neighbors = '1'
    boundary_new = 'interface'
    bottom_left = '-0.1 -0.1 0'
    top_right = '0.6 1.1 0'
  []
[]

[Outputs]
  exodus = true
[]
