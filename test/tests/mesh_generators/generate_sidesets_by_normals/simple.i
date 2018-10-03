[MeshGenerators]
  [./fmg]
    type = FileMeshGenerator
    file = twoblocks.e
  []

  [./generate_sidesets]
    type = GenerateAllSideSetsByNormals
    input = fmg
  []
[]

[Mesh]
  type = MeshGeneratorMesh
[]

[Outputs]
  exodus = true
[]
