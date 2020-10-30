[Mesh]
  [file]
    type = FileMeshGenerator
    file = 4ElementJunction.e
  []

  [breakmesh]
    type = BreakMeshByBlockGenerator
    input = file
  []
[]
