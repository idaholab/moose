[Mesh]
  [./layer1]
    type = GeneratedMeshGenerator
    dim = 3
    nx = 10
    xmax = 10
    ny = 10
    ymax = 10
    nz = 5
    zmax = 5
  []

  [./layer2]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 10
    xmax = 10
    ny = 10
    ymax = 10
  []

  [./stack]
    type = StackGenerator
    dim = 3
    inputs = 'layer1 layer2'
  []
[]

[Outputs]
  exodus = true
[]
