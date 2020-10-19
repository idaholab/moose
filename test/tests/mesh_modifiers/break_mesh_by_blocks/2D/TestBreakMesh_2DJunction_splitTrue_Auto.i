[Mesh]
  [file]
    type = FileMeshGenerator
    file = 4ElementJunction.e
  []

  [breakmesh]
    type = BreakMeshByBlockGenerator
    input = file
    split_interface = true
  []
[]

# This input file is intended to be run with the "--mesh-only" option so
# no other sections are required
