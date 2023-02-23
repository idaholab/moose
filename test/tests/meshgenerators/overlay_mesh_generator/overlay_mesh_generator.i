[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 3
    ny = 3
    xmin = -1
    xmax = 4
    ymin = -1
    ymax = 2.2
    output = true
  []
  [bcg]
    type = OverlayMeshGenerator
    input = 'gmg'
    dim = 2
    nx = 6
    ny = 6
  []
[]

[Outputs]
 exodus = true
[]


