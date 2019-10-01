[Mesh]
  [./gmg]
    type = GeneratedMeshGenerator
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

  [./subdomains]
    type = ElementSubdomainIDGenerator
    input = gmg
    subdomain_ids = '0 0 0 0
                     0 0 0 0
                     1 1 0 0
                     2 2 2 2
                     3 3 0 0
                     3 3 0 0
                     1 1 0 0
                     0 0 0 0'
  []
  [./interface]
    type = SideSetsAroundSubdomainGenerator
    input = subdomains
    block = '1 2 3'
    new_boundary = 'to0'
  []
[]

[Outputs]
  exodus = true
[]
