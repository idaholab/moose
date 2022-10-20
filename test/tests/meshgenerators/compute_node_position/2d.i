[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 11
    ny = 11
    xmin = -1
    ymin = -2
    xmax = 9
    ymax = 8
  []
  [node_pos]
    type = ParsedNodeTransformGenerator
    input = gen
    x_function = 'if(x < 1 | y < 2, x, x + (y-2)*0.2)'
    y_function = 'if(x < 2 | y < 1, y, y + (y-1)*(x-2) * 0.1)'
  []
[]
