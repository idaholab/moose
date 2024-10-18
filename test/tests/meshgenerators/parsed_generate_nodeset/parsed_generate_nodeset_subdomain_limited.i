[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 3
    nx = 2
    ny = 3
    nz = 3
    xmax = 3
    ymax = 3
    zmax = 3
  []

  [subdomains]
    type = ParsedSubdomainMeshGenerator
    input = gmg
    combinatorial_geometry = 'x < 1 & y > 1 & y < 2'
    block_id = 1
  []

  [nodeset]
    type = ParsedGenerateNodeset
    input = subdomains
    expression = 'z < 1'
    included_subdomains = '1'
    new_nodeset_name = interior
  []
[]

[Outputs]
  exodus = true
[]
