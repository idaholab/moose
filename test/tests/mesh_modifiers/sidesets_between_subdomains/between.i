[Mesh]
  type = FileMesh
  file = twoblocks.e
[]

[MeshModifiers]
  [./extrude]
    type = SideSetsBetweenSubdomains
    master_block = 'left'
    paired_block = 'right'
    boundary = 'in_between'
  [../]
[]

# This input file is intended to be run with the "--mesh-only" option so
# no other sections are required
