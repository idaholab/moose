[Mesh]
  [./fmg]
    type = FileMeshGenerator
    file = twoblocks.e
  []

  [./block_1]
    type = SideSetsAroundSubdomainGenerator
    input = fmg
    block = 'left'
    new_boundary = 'hull_1'
  []

  [./block_2]
    type = SideSetsAroundSubdomainGenerator
    input = block_1
    block = 'right'
    new_boundary = 'hull_2'
  []
[]

[Outputs]
  exodus = true
[]
