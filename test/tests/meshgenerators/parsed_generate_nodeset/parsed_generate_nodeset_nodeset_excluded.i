[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 3
    nx = 3
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
    expression = 'z < 1.6'
    included_subdomains = '1'
    new_nodeset_name = interior
  []

  [nodeset_further_limited]
    type = ParsedGenerateNodeset
    input = nodeset
    expression = 'z > -1'
    included_subdomains = '1'
    excluded_nodesets = 'interior'
    new_nodeset_name = 'none_from_interior'
  []
[]

[Outputs]
  exodus = true
[]
