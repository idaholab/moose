[MeshGenerators]
  [./gmg]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 2
    ny = 2
    nz = 0
    zmin = 0
    zmax = 0
    elem_type = QUAD4
  []

  [./subdomain_id]
    type = ElementSubdomainIDGenerator
    input = gmg
    element_ids = '1 2 3'
    subdomain_ids = '1 1 1'
  []
[]

[Outputs]
  exodus = true
[]
