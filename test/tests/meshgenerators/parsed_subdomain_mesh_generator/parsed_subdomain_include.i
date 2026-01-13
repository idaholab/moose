[Mesh]
  [fmg]
    type = FileMeshGenerator
    file = 'gold/parsed_subdomain_mg_in.e'
  []
  [sd_included]
    type = ParsedSubdomainMeshGenerator
    input = fmg
    combinatorial_geometry = 'y>x-0.01'
    block_id = 3
    included_subdomains = '0 2'
  []
[]
