[Mesh]
  [outer_bdy]
    type = ParsedCurveGenerator
    x_formula = 'r*cos(t)'
    y_formula = 'r*sin(t)'
    section_bounding_t_values = '${fparse 0.0} ${fparse pi} ${fparse 2.0*pi}'
    constant_names = 'r'
    constant_expressions = '1.0'
    nums_segments = '8 16'
    is_closed_loop = true
  []
  [hole_1]
    type = PolyLineMeshGenerator
    points = '-0.5 -0.1 0.0
              -0.3 -0.1 0.0
              -0.3 0.1 0.0
              -0.5 0.1 0.0'
    loop = true
  []
  [hole_2]
    type = PolyLineMeshGenerator
    points = '0.3 -0.1 0.0
              0.5 -0.1 0.0
              0.5 0.1 0.0
              0.3 0.1 0.0'
    loop = true
    num_edges_between_points = 3
  []
  [triang]
    type = XYDelaunayGenerator
    boundary = 'outer_bdy'
    holes = 'hole_1
             hole_2'
    refine_boundary=false
    refine_holes="false false"
    use_auto_area_func="true"
  []
[]
