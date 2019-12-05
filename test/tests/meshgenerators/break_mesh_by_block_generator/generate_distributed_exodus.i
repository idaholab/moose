[Mesh]
  [./msh]
    type = GeneratedMeshGenerator
    nx = 3
    ny = 3
    dim = 2
    xmax = 3
    ymax = 3
  []

  [./subdomain_0]
      type = SubdomainBoundingBoxGenerator
      input = msh
      bottom_left = '1 1 0'
      top_right = '2 2 0'
      block_id = 0
      location = OUTSIDE
  []
  [./subdomain_1]
      type = SubdomainBoundingBoxGenerator
      input = subdomain_0
      bottom_left = '1 1 0'
      top_right = '2 2 0'
      block_id = 1
  []
  [./subdomain_2]
      type = SubdomainBoundingBoxGenerator
      input = subdomain_1
      bottom_left = '2 2 0'
      top_right = '3 3 0'
      block_id = 2
  []
[]

[Outputs]
  exodus = true
[]
