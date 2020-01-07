[Mesh]
  [./layer1]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 10
    xmax = 10
    ny = 5
    ymax = 5
  []

  [./layer2]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 10
    xmax = 10
    ny = 5
    ymax = 10
  []

  [./gmg]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 10
    xmax = 10
    ny = 10
    ymax = 10
  []

  [./bounding_box1]
    type = SubdomainBoundingBoxGenerator
    input = gmg
    top_right = '10 10 0'
    bottom_left = '0 7 0'
    block_id = 1
  []

  [./layer3]
    type = SubdomainBoundingBoxGenerator
    input = bounding_box1
    top_right = '3 3 0'
    bottom_left = '0 0 0'
    block_id = 2
  []

  [./layer4]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 10
    xmax = 10
    ny = 2
    ymax = 2
  []

  [./stack_them]
    type = StackGenerator
    inputs = 'layer1 layer2 layer3 layer4'
    dim = 2
    bottom_height = 3
  []
[]

[Outputs]
  exodus = true
[]
