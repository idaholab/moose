[MeshGenerators]
  [./fmg]
    type = FileMeshGenerator
    file = 4ElementJunction.e
  []

  [./breakmesh]
    type = BreakMeshByBlockGenerator
    input = fmg
    split_interface = true
  []
[]

[Mesh]
  type = MeshGeneratorMesh
[]

[Outputs]
  exodus = true
[]
