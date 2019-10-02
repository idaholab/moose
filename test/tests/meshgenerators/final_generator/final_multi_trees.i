[Mesh]
  [./gmg]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 5
    ny = 5
    xmax = 1
    ymax = 1
  []

  [./subdomain_lower]
    type = SubdomainBoundingBoxGenerator
    input = gmg
    bottom_left = '0.2 0.2 0'
    block_id = 1
    top_right = '0.4 0.4 0'
  []

  # Independent Tree of Generators
  [./gmg2]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 5
    ny = 5
    xmax = 1
    ymax = 1
  []

  [./subdomain_upper]
    type = SubdomainBoundingBoxGenerator
    input = gmg2
    bottom_left = '0.6 0.6 0'
    block_id = 1
    top_right = '0.8 0.8 0'
  []
[]
