[Mesh]
  inactive = 'refine'
  [file]
    type = FileMeshGenerator
    file = 3dmultiblock.e
  []
  [blockToMesh]
    type = BlockToMeshConverterGenerator
    input = file
    target_blocks = "1 12 2 3 5 6"
  []
  [refine]
    type = RefineBlockGenerator
    input = file
    block = '12'
    refinement = '1'
  []
[]
