[Mesh]
  [accg]
    type = FileMeshGenerator
    file = 'hex_prism_2d.e'
  []
  [extrude]
    type = AdvancedExtruderGenerator
    input = accg
    heights = '0.4 0.8 1.2'
    num_layers = '1 2 3'
    direction = '0 0 1'
    bottom_boundary = '200'
    top_boundary = '300'
    subdomain_swaps = '10 11 15 16;
                       10 12 15 17;
                       10 13 15 18'
  []
  [convert]
    type = ElementsToSimplicesConverter
    input = extrude
  []

  # Simplices conversion currently depends on element id numbering
  allow_renumbering = false
[]
