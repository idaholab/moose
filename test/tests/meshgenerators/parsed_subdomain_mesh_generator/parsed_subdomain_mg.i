[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 10
    ny = 10
    xmax = 1
    ymax = 1
  []

  [subdomains]
    type = ParsedSubdomainMeshGenerator
    input = gmg
    combinatorial_geometry = 'x > 0.1 & x < 0.9 & y > 0.1 & y < 0.9'
    block_id = 1
  []

  [subdomains2]
    type = ParsedSubdomainMeshGenerator
    combinatorial_geometry = 'x < 0.5 & y < 0.5'
    excluded_subdomains = '0'
    block_id = 2
    input = subdomains
  []
[]

[Outputs]
  exodus = true
[]
