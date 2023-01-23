[Mesh]
  [pcg]
    type = ParsedCurveGenerator
    x_formula = '1.0*cos(t)'
    y_formula = 'y1:=1.0*sin(t);
                 y2:=1.5*sin(t);
                 if(t<pi,y1,y2)'
    section_bounding_t_values = '0.0 ${fparse pi} ${fparse 2.0*pi}'
    nums_segments = '10 10'
    constant_names = 'pi'
    constant_expressions = '${fparse pi}'
    is_closed_loop = true
  []
[]
