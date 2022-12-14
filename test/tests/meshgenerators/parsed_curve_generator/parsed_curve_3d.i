[Mesh]
  [pcg]
    type = ParsedCurveGenerator
    x_formulus = 'cos(t)'
    y_formulus = 'sin(t)'
    z_formulus = 't'
    critical_t_series = '0 ${fparse 4*pi}'
    nums_segments = 24
  []
[]
