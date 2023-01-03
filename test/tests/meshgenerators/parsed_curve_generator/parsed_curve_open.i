[Mesh]
  [pcg]
    type = ParsedCurveGenerator
    x_formulus = 't'
    y_formulus = 'log10(1+9*t)'
    critical_t_series = '0 1'
    nums_segments = 8
  []
[]
