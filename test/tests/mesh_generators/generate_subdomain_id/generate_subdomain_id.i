[MeshGenerators]
  [./gmg]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 10
    ny = 10
  []

  [./generate_id]
    type = GenerateSubdomainID
    input = gmg
    subdomain_id = 3
  []
[]

[Mesh]
  type = MeshGeneratorMesh
[]

[Outputs]
  exodus = true
[]
