[Mesh]
  [pcg]
    type = ParsedCurveGenerator
    x_formula = 't'
    y_formula = 'log10(1+9*t)'
    section_bounding_t_values = '0 1'
    nums_segments = 8
  []
[]
