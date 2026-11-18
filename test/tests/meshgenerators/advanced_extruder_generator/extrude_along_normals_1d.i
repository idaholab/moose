[Mesh]
  # A straight 1D mesh ...
  [gmg]
    type = GeneratedMeshGenerator
    dim = 1
    nx = 6
    xmin = -1
    xmax = 1
  []
  # ... bent into an arc in the x-y plane, so that the curve normal varies
  # from node to node
  [arc]
    type = ParsedNodeTransformGenerator
    input = gmg
    y_function = '0.5 * (1 - x * x)'
  []
  # Grow boundary layers off the curve by extruding each node along the
  # averaged normal of the elements it is connected to
  [extrude]
    type = AdvancedExtruderGenerator
    input = arc
    extrude_along_node_normals = true
    heights = '0.3'
    num_layers = '3'
    biases = '1.5'
  []
[]
