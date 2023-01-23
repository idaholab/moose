[Mesh]
  [pcg1]
    type = ParsedCurveGenerator
    x_formula = 't'
    y_formula = 'log10(1+9*t)'
    section_bounding_t_values = '0 1.5'
    nums_segments = 7
  []
  [pcg2]
    type = ParsedCurveGenerator
    x_formula = 'cos(t)+1.0'
    y_formula = 'sin(t)'
    section_bounding_t_values = '${fparse -pi/2.0} ${fparse 0.0}'
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
