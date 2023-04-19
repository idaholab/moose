[Mesh]
    [gmg]
      type = GeneratedMeshGenerator
      dim = 2
      nx = 6
      ny = 6
      nz = 0
      zmin = 0
      zmax = 0
      elem_type = QUAD4
    []

    [extrude]
      type = AdvancedExtruderGenerator
      input = gmg
      heights = '2 1 2'
      num_layers = '5 3 5'
      biases = '1.6 1.0 0.625'
      direction = '0 0 1'
      bottom_sideset = '4'
      top_sideset = '5'
      subdomain_swaps = '0 1;
                         0 2;
                         0 3'
    []
  []
