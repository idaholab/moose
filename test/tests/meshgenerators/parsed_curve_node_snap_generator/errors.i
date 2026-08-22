[Mesh]
  [circle]
    type = ParsedCurveGenerator
    x_formula = 'cos(t)'
    y_formula = 'sin(t)'
    section_bounding_t_values = '${fparse 0.0} ${fparse pi} ${fparse 2.0*pi}'
    nums_segments = '4 4'
    is_closed_loop = true
  []
  [triang]
    type = XYFrontalDelaunayGenerator
    boundary = 'circle'
    refine_boundary = false
    desired_area = 0.2
    output_boundary = 'circumference'
  []
  [to_quad]
    type = TriToQuadGenerator
    input = triang
    all_quad = true
  []
  [snap]
    type = ParsedCurveNodeSnapGenerator
    input = to_quad
    boundary = 'circumference'
    curve_generator = circle
  []
[]
