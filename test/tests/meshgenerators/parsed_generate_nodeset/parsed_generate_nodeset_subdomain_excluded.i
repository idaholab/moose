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
    subdomain_ids = '0 0
                     1 1
                     2 2
                     0 0
                     1 1
                     2 2
                     0 0
                     2 2
                     3 3'
  []

  [nodeset]
    type = ParsedGenerateNodeset
    input = gmg
    expression = '1'
    included_subdomains = 2
    excluded_subdomains = 3
    new_nodeset_name = test
  []
[]

[Outputs]
  exodus = true
[]
