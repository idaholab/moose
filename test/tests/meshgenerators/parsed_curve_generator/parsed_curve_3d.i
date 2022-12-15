[Mesh]
  [pcg]
    type = ParsedCurveGenerator
    x_formula = 'cos(t)'
    y_formula = 'sin(t)'
    z_formula = 't'
    critical_t_series = '0 ${fparse 4*pi}'
    nums_segments = 24
  []
[]
