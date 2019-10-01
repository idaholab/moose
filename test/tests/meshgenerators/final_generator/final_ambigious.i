[Mesh]
  # This parameter is supplied through the test specification
  # final_generator = subdomain_lower

  [./gmg]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 5
    ny = 5
    xmax = 1
    ymax = 1
    #uniform_refine = 2
  []

  [./subdomain_lower]
    type = SubdomainBoundingBoxGenerator
    input = gmg
    bottom_left = '0.2 0.2 0'
    block_id = 1
    top_right = '0.4 0.4 0'
  []

  [./subdomain_upper]
    type = SubdomainBoundingBoxGenerator
    input = gmg
    bottom_left = '0.6 0.6 0'
    block_id = 1
    top_right = '0.8 0.8 0'
  []
[]
