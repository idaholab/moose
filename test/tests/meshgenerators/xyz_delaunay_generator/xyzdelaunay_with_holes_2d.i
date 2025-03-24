[Mesh]
  [fmg]
    type = FileMeshGenerator
    file = 'gold/2d_cube.e'
  []
  [outer_bdy]
    type = ParsedNodeTransformGenerator
    input = fmg
    x_function = "x"
    y_function = "y"
    z_function = "z+x*y*z"
  []
  [hole_1]
    type = ParsedNodeTransformGenerator
    input = fmg
    x_function = ".25+.125*x"
    y_function = ".25+.125*y"
    z_function = ".25+.125*z"
  []
  [hole_2]
    type = ParsedNodeTransformGenerator
    input = fmg
    x_function = ".75+.125*x"
    y_function = ".75+.125*y"
    z_function = ".75+.125*z"
  []
  [triang]
    type = XYZDelaunayGenerator
    boundary = 'outer_bdy'
    holes = 'hole_1
             hole_2'
    # Let NetGen know interior points are okay
    desired_volume = 100000
  []
[]
