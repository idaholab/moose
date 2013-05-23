[Mesh]
  type = FileMesh
  file = twoblocks.e
[]

[MeshModifiers]
  [./block_1]
    type = SideSetsAroundSubdomain
    block = 'left'
    boundary = 'hull_1'
  [../]

  [./block_2]
    type = SideSetsAroundSubdomain
    block = 'right'
    boundary = 'hull_2'
  [../]
[]

# This input file is intended to be run with the "--mesh-only" option so
# no other sections are required
