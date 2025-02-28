[Mesh]
  [cmg]
    type = CartesianMeshGenerator
    dim = 2
    ix = 3
    iy = 3
    dx = 1
    dy = 1
  []
  [layer]
    type = BoundaryLayerSubdomainGenerator
    input = 'cmg'
    boundaries = 'left top'
    block_name = 'new'
  []
[]
