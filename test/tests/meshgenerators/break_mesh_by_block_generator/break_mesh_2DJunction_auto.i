[MeshGenerators]
  [./fmg]
    type = FileMeshGenerator
    file = 4ElementJunction.e
  []

  [./breakmesh]
    type = BreakMeshByBlockGenerator
    input = fmg
  []
[]

[Mesh]
  type = MeshGeneratorMesh
[]

[Outputs]
  exodus = true
[]
