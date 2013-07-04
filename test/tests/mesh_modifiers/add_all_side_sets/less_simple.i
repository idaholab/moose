[Mesh]
  type = FileMesh
  file = reactor.e
[]

[MeshModifiers]
  [./block_1]
    type = AddAllSideSetsByNormals
  [../]
[]

# This input file is intended to be run with the "--mesh-only" option so
# no other sections are required
