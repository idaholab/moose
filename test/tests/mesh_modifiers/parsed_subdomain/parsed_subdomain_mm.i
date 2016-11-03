[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 10
  ny = 10
  xmax = 1
  ymax = 1
  uniform_refine = 2
[]

[MeshModifiers]
  [./subdomains]
    type = ParsedSubdomainMeshModifier
    combinatorial_geometry = 'x > 0.1 & x < 0.9 & y > 0.1 & y < 0.9'
    block_id = 1
  [../]
  [./subdomains2]
    type = ParsedSubdomainMeshModifier
    combinatorial_geometry = 'x < 0.5 & y < 0.5'
    excluded_subdomain_ids = '0'
    block_id = 2
    depends_on = subdomains
  [../]
[]

# This input file is intended to be run with the "--mesh-only" option so
# no other sections are required
