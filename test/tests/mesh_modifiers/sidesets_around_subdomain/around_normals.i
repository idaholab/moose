[Mesh]
  [file]
    type = FileMeshGenerator
    file = twoblocks.e
  []

  [top_block_1]
    type = SideSetsAroundSubdomainGenerator
    input = file
    block = 'left'
    new_boundary = 'top_of_left_block'
    normal = '0 0 1'
  []

  [bottom_block_2]
    type = SideSetsAroundSubdomainGenerator
    input = top_block_1
    block = 'right'
    new_boundary = 'bottom_of_right_block'
    normal = '0 0 -1'
  []

  [right_block_1]
    type = SideSetsAroundSubdomainGenerator
    input = bottom_block_2
    block = 'left'
    new_boundary = 'right_of_left_block'
    normal = '1 0 0'
  []

  [right_block_2]
    type = SideSetsAroundSubdomainGenerator
    input = right_block_1
    block = 'right'
    new_boundary = 'right_of_right_block'
    normal = '1 0 0'
  []

[]

# This input file is intended to be run with the "--mesh-only" option so
# no other sections are required
