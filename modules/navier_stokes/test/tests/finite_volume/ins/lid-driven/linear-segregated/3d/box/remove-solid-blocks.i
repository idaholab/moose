[Mesh]
  [file]
    type = FileMeshGenerator
    file = zach-mesh_in.e
  []
  [delete]
    type = BlockDeletionGenerator
    input = file
    block = '0 2'
  []
[]

