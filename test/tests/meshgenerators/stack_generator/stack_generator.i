[MeshGenerators]
  [./layer1]
    type = GeneratedMeshGenerator
    dim = 3
    nx = 10
    xmax = 10
    ny = 10
    ymax = 10
    nz = 3
    zmax = 3
  []

  [./layer2]
    type = GeneratedMeshGenerator
    dim = 3
    nx = 10
    xmax = 10
    ny = 10
    ymax = 10
    nz = 5
    zmax = 5
  []

  [./layer3]
    type = SubdomainBoundingBoxGenerator
    input = layer2
    bottom_left = '3 3 3'
    top_right = '5 5 5'
    block_id = 2
  []

  [./layer4]
    type = GeneratedMeshGenerator
    dim = 3
    nx = 10
    xmax = 10
    ny = 10
    ymax = 10
    nz = 5
    zmax = 2
  []

  [./stack]
    type = StackGenerator
    inputs = 'layer1 layer2 layer3 layer4'
    bottom_height = 4
  []


[]

[Mesh]
  type = MeshGeneratorMesh
[]

[Outputs]
  exodus = true
[]
