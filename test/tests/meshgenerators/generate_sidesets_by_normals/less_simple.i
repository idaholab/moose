[MeshGenerators]
  [./fmg]
    type = FileMeshGenerator
    file = reactor.e
  []

  [./generate_sidesets]
    type = GenerateAllSideSetsByNormals
    input = fmg
  []
[]

[Outputs]
  exodus = true
[]
