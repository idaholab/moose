[MeshGenerators]
  [./gmg]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 176
    ny = 287
  []

  [./image]
    type = ImageSubdomainGenerator
    input = gmg
    file = kitten.png
    threshold = 100
  []
[]

[Mesh]
  type = MeshGeneratorMesh
[]

[Outputs]
  exodus = true
[]
