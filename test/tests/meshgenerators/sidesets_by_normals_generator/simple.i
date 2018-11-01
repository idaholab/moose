[MeshGenerators]
  [./fmg]
    type = FileMeshGenerator
    file = twoblocks.e
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
