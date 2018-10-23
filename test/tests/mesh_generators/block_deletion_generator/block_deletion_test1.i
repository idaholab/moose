[MeshGenerators]
  [./gmg]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 4
    ny = 4
    xmin = 0
    xmax = 4
    ymin = 0
    ymax = 4
  []

  [./generate_id]
    type = GenerateElementSubdomainID
    input = gmg
    element_ids = '5 6 9 10'
    subdomain_ids = '1 1 1 1'
  []

  [./deletion]
    type = BlockDeletionGenerator
    input = generate_id
    block_id = 1
  []
[]

[Mesh]
  type = MeshGeneratorMesh
[]

[Outputs]
  exodus = true
[]
