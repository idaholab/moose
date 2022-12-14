[Mesh]
  [pcg1]
    type = ParsedCurveGenerator
    x_formulus = 't'
    y_formulus = 'log10(1+9*t)'
    critical_t_series = '0 1.5'
    nums_segments = 7
  []
  [pcg2]
    type = ParsedCurveGenerator
    x_formulus = 'cos(t)+1.0'
    y_formulus = 'sin(t)'
    critical_t_series = '${fparse -pi/2.0} ${fparse 0.0}'
    nums_segments = 5
  []
  [fbcg]
    type = FillBetweenCurvesGenerator
    input_mesh_1 = pcg1
    input_mesh_2 = pcg2
    num_layers = 3
    bias_parameter = 0.0
  []
[]
