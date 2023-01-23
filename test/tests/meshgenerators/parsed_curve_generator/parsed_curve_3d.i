[Mesh]
  [pcg]
    type = ParsedCurveGenerator
    x_formula = 'cos(t)'
    y_formula = 'sin(t)'
    z_formula = 't'
    section_bounding_t_values = '0 ${fparse 4*pi}'
    nums_segments = 24
  []
[]
