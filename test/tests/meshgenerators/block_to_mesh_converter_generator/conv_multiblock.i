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
  # added to preserve the old gold file when all blocks were merged into block 0
  [change_block_names]
    type = RenameBlockGenerator
    input = blockToMesh
    old_block = '1 12 2 3 5 6'
    new_block = '0 0  0 0 0 0'
  []
[]
