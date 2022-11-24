[Mesh]
  [square]
    type = GeneratedMeshGenerator
    dim=2
  []
  [blockToMesh]
    type = BlockToMeshConverterGenerator
    input = square
    target_blocks = "0"
  []
[]
