[Mesh]
  [gen]
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
    input = gen
    combinatorial_geometry = 'x < 1 & y > 1 & y < 2'
    block_id = 1
  []
  [sideset]
    type = ParsedGenerateSideset
    input = subdomains
    combinatorial_geometry = 'z < 1'
    included_subdomain_ids = '1'
    normal = '1 0 0'
    new_sideset_name = interior
  []
[]

# This input file is intended to be run with the "--mesh-only" option so
# no other sections are required
