[Mesh]
  [./gmg]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 10
    ny = 10
  []

  [./generate_id]
    type = SubdomainIDGenerator
    input = gmg
    subdomain_id = 3
  []
[]

[Outputs]
  exodus = true
[]
