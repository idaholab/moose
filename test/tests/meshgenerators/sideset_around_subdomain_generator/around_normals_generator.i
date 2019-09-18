[Mesh]
  [./fmg]
    type = FileMeshGenerator
    file = twoblocks.e
  []

  [./top_block1]
    type = SideSetsAroundSubdomainGenerator
    input = fmg
    block = 'left'
    new_boundary = 'top_of_left_block'
    normal = '0 0 1'
  []

  [./bottom_block2]
    type = SideSetsAroundSubdomainGenerator
    input = top_block1
    block = 'right'
    new_boundary = 'bottom_of_right_block'
    normal = '0 0 -1'
  []

  [./right_block1]
    type = SideSetsAroundSubdomainGenerator
    input = bottom_block2
    block = 'left'
    new_boundary = 'right_of_left_block'
    normal = '1 0 0'
  []

  [./right_block2]
    type = SideSetsAroundSubdomainGenerator
    input = right_block1
    block = 'right'
    new_boundary = 'right_of_right_block'
    normal = '1 0 0'
  []

[]

[Outputs]
  exodus = true
[]
