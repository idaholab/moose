[MeshGenerators]
  [./gmg]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 10
    ny = 10
  []

  [./lower_d_block]
    type = LowerDBlockFromSidesetGenerator
    input = gmg
    new_block_id = 10
    sidesets = '0 0 1 2 3'
  []
[]

[Mesh]
  type = MeshGeneratorMesh
[]

[Outputs]
  exodus = true
[]
