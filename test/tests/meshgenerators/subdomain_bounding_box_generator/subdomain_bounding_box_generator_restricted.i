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
    type = SubdomainBoundingBoxGenerator
    input = gmg
    bottom_left = '0.1 0.1 0'
    block_id = 1
    top_right = '0.9 0.9 0'
  []

  [subdomains2]
    type = SubdomainBoundingBoxGenerator
    input = subdomains
    bottom_left = '0.2 0 0'
    block_id = 2
    top_right = '1 0.6 0'
    restricted_subdomains = 0
  []
[]
