[Mesh]
  [file]
    type = FileMeshGenerator
    file = multiblock.e
  []
  [blockToMesh]
    type = BlockToMeshConverterGenerator
    input = file
    target_blocks = "1 2"
  []
[]
