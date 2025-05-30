[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 3
    nx = 1
    ny = 1
    nz = 1
  []
  [outer_bdy]
    type = ParsedNodeTransformGenerator
    input = gmg
    x_function = "x"
    y_function = "y"
    z_function = "z+x*y*z"
  []
  [hole_1d]
    type = ParsedCurveGenerator
    x_formula = 'cos(t)'
    y_formula = 'sin(t)'
    section_bounding_t_values = '0.0 ${fparse 2.0*pi}'
    nums_segments = '10'
    constant_names = 'pi'
    constant_expressions = '${fparse pi}'
    is_closed_loop = true
  []
  [triang]
    type = XYZDelaunayGenerator
    boundary = 'outer_bdy'
    holes = 'hole_1d'
  []
[]
