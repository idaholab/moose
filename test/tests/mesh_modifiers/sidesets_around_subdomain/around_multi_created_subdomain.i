[Mesh]
  type = GeneratedMesh
  dim = 3
  xmin = 0
  xmax = 4
  nx = 4
  ymin = 0
  ymax = 4
  ny = 4
  zmin = 0
  zmax = 2
  nz = 2
[]

[MeshModifiers]
  [./subdomains]
    type = AssignElementSubdomainID
    subdomain_ids = '0 0 0 0
                     0 0 0 0
                     1 1 0 0
                     2 2 2 2
                     3 3 0 0
                     3 3 0 0
                     1 1 0 0
                     0 0 0 0'
  [../]
  [./interface]
    type = SideSetsAroundSubdomain
    block = '1 2 3'
    new_boundary = 'to0'
    depends_on = subdomains
  [../]
[]

# This input file is intended to be run with the "--mesh-only" option so
# no other sections are required
