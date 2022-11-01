[Mesh]
  [file]
    type = FileMeshGenerator
    file = mixedDimMesh.e
  []
  [blockToMesh]
    type = BlockToMeshConverterGenerator
    input = file
    target_blocks = "1"
  []
[]
