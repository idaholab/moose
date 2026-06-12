[Mesh]
  # A flat 2D quad mesh ...
  [gmg]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 4
    ny = 4
    xmin = -1
    xmax = 1
    ymin = -1
    ymax = 1
  []
  # ... warped into a curved 3D surface (a gentle dome), so that the surface
  # normal varies from node to node
  [dome]
    type = ParsedNodeTransformGenerator
    input = gmg
    z_function = '0.5 * (1 - 0.25 * (x * x + y * y))'
  []
  # Grow boundary layers off the surface by extruding each node along the
  # averaged normal of the elements it is connected to
  [extrude]
    type = AdvancedExtruderGenerator
    input = dome
    extrude_along_node_normals = true
    heights = '0.3'
    num_layers = '3'
    biases = '1.5'
  []
[]
