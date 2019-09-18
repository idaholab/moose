[Mesh]
  [read]
    type = FileMeshGenerator
    file = reactor.e
  []
  [block_1]
    type = AllSideSetsByNormalsGenerator
    input = read
  []
[]

# This input file is intended to be run with the "--mesh-only" option so
# no other sections are required
