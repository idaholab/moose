[MeshGenerators]
  [./fmg]
    type = FileMeshGenerator
    file = reactor.e
  []

  [./generate_sidesets]
    type = AllSideSetsByNormalsGenerator
    input = fmg
  []
[]

[Mesh]
  type = MeshGeneratorMesh
[]

[Outputs]
  exodus = true
[]
