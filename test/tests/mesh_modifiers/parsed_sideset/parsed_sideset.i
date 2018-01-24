[Mesh]
  type = GeneratedMesh
  dim = 3
  nx = 3
  ny = 3
  nz = 3
  xmax = 3
  ymax = 3
  zmax = 3
[]

[MeshModifiers]
  [./subdomains]
    type = ParsedSubdomainMeshModifier
    combinatorial_geometry = 'x < 1 & y > 1 & y < 2'
    block_id = 1
  [../]
  [./sideset]
    type = ParsedAddSideset
    combinatorial_geometry = 'z < 1'
    included_subdomain_ids = '1'
    normal = '1 0 0'
    new_sideset_name = interior
    depends_on = subdomains
  [../]
[]

# This input file is intended to be run with the "--mesh-only" option so
# no other sections are required
