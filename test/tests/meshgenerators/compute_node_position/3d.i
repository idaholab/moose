[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 3
    nx = 5
    ny = 5
    nz = 10
    xmin = -0.5
    ymin = -0.5
    xmax = 0.5
    ymax = 0.5
    zmax = 3
  []
  [node_pos]
    type = ParsedNodeTransformGenerator
    input = gen
    x_function = 'x*cos(z) + y*sin(z)'
    y_function = '-x*sin(z) + y*cos(z)'
    z_function = 'z^0.8'
  []
[]
