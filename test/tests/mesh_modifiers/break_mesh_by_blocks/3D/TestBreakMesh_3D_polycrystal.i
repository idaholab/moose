[Mesh]
  [file]
    type = FileMeshGenerator
    file = poly.msh
  []

  [breakmesh]
    type = BreakMeshByBlockGenerator
    input = file
    split_interface = false
  []
[]
