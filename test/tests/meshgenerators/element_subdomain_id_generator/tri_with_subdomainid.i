[MeshGenerators]
  [./gmg]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 2
    ny = 2
    nz = 0
    zmin = 0
    zmax = 0
    elem_type = TRI3
  []

  [./subdomain_id]
    type = ElementSubdomainIDGenerator
    input = gmg
    subdomain_ids = '0 1  1 1
                     1 1  1 0'
  []
[]

[Outputs]
  exodus = true
[]
