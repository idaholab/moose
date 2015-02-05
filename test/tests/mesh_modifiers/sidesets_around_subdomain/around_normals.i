[Mesh]
  type = FileMesh
  file = twoblocks.e
[]

[MeshModifiers]
  [./top_block_1]
    type = SideSetsAroundSubdomain
    block = 'left'
    new_boundary = 'top_of_left_block'
    normal = '0 0 1'
  [../]

  [./bottom_block_2]
    type = SideSetsAroundSubdomain
    block = 'right'
    new_boundary = 'bottom_of_right_block'
    normal = '0 0 -1'
  [../]

  [./right_block_1]
    type = SideSetsAroundSubdomain
    block = 'left'
    new_boundary = 'right_of_left_block'
    normal = '1 0 0'
  [../]

  [./right_block_2]
    type = SideSetsAroundSubdomain
    block = 'right'
    new_boundary = 'right_of_right_block'
    normal = '1 0 0'
  [../]

[]

# This input file is intended to be run with the "--mesh-only" option so
# no other sections are required
