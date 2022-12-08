[Mesh]
  [pcg]
    type = ParsedCurveGenerator
    x_formulus = 'r*cos(t)'
    y_formulus = 'y1:=r*sin(t);
                  y2:=b*sin(t);
                  if(t<pi,y1,y2)'
    critical_t_series = '${fparse 0.0} ${fparse pi} ${fparse 2.0*pi}'
    constant_names = 'pi r b'
    constant_expressions = '${fparse pi} 1.0 1.5'
    nums_segments = '10 10'
    is_closed_loop = true
  []
  [hole1]
    type = ParsedCurveGenerator
    constant_names = 'th a b xs ys'
    constant_expressions = '${fparse pi/4.0} 0.25 0.5 0.1 0.3'
    x_formulus = 'x0:=a*cos(t);
                  y0:=b*sin(t);
                  cos(th)*x0-sin(th)*y0+xs'
    y_formulus = 'x0:=a*cos(t);
                  y0:=b*sin(t);
                  sin(th)*x0+cos(th)*y0+ys'
    critical_t_series = '0.0 ${fparse 2.0*pi}'
    nums_segments = 18
    is_closed_loop = true
  []
  [hole2]
    type = ParsedCurveGenerator
    constant_names = 'th a xs ys'
    constant_expressions = '${fparse pi/2.0} 0.3 -0.1 -0.8'
    x_formulus = 'x0:=a*(1+cos(t))*cos(t);
                  y0:=a*(1+cos(t))*sin(t);
                  cos(th)*x0-sin(th)*y0+xs'
    y_formulus = 'x0:=a*(1+cos(t))*cos(t);
                  y0:=a*(1+cos(t))*sin(t);
                  sin(th)*x0+cos(th)*y0+ys'
    critical_t_series = '0 ${fparse pi} ${fparse 2.0*pi}'
    nums_segments = '9 9'
    is_closed_loop = true
  []
  [xydg]
    type = XYDelaunayGenerator
    boundary = 'pcg'
    holes = 'hole1 hole2'
    add_nodes_per_boundary_segment = 1
    refine_boundary = false
    desired_area = 0.03
  []
[]
