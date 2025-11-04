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
  [hole_1_2d]
    type = ParsedNodeTransformGenerator
    input = fmg
    x_function = ".25+.125*x"
    y_function = ".25+.125*y"
    z_function = ".25+.125*z"
  []
  [hole_1]
    type = ElementGenerator
    input = 'hole_1_2d'
    elem_type = TET4
    nodal_positions = '0 0 0
                       0 1 0
                       1 0 0
                       0 0 1'
    element_connectivity = '0 1 2 3'
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
