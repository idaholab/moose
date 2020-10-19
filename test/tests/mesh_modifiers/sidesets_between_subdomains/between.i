[Mesh]
  [file]
    type = FileMeshGenerator
    file = twoblocks.e
  []

  [extrude]
    type = SideSetsBetweenSubdomainsGenerator
    input = file
    primary_block = 'left'
    paired_block = 'right'
    new_boundary = 'in_between'
  []
[]

# This input file is intended to be run with the "--mesh-only" option so
# no other sections are required
