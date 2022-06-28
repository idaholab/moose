[Mesh]
  [file]
    type = FileMeshGenerator
    file = 3dmultiblock.e
  []
  [blockToMesh]
    type = BlockToMeshConverterGenerator
    input = file
    target_blocks = "1 12 2 3 5 6"
  []
[]
