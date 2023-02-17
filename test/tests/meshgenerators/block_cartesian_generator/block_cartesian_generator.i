[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 3
    ny = 3
    xmin = 0
    xmax = 4
    ymin = 0
    ymax = 2.2
  []
  [bcg]
    type = BlockCartesianGenerator
    input = 'gmg'
    dim = 2
    nx = 6
    ny = 6
  []
[]

[Outputs]

[]

