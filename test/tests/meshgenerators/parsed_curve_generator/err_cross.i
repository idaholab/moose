[Mesh]
  [pcg]
    type = ParsedCurveGenerator
    x_formula = 'if(t<2*pi,cos(t)-1.0,cos(t+pi)+1.0)'
    y_formula = 'sin(t)'
    section_bounding_t_values = '0 ${fparse 2*pi} ${fparse 4*pi}'
    constant_names = 'pi'
    constant_expressions = '${fparse pi}'
    nums_segments = '16 16'
  []
[]
