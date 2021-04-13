[Mesh]
  [cmg]
    type = CartesianMeshGenerator
    dim = 2
    dx = '1 1'
    dy = '2 2'
    subdomain_id = '0 1 0 0'
  []

  [sideset]
    type = ParsedGenerateSideset
    input = cmg
    combinatorial_geometry = 'abs(x - 1) < 1e-6'
    included_neighbor_ids = '1'
    new_sideset_name = interior
  []
[]

[Outputs]
  exodus = true
[]
