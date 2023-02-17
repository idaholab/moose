[Mesh]
  [left]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 16
    ny = 16
    xmin = -3
    xmax = 0
    ymin = -5
    ymax = 5
    output = true
  []
  [right]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 100
    ny = 100
    xmin = 3
    xmax = 6
    ymin = -5
    ymax = 5
  []

  [left_and_right]
    type = MeshCollectionGenerator
    inputs = 'left right'
  []
[]

