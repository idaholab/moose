[Mesh]
  [pmg]
    type = ParsedCurveGenerator
    x_formula = 't1:=t;
                   t2:=t-1;
                   t3:=t-2;
                   t4:=t-3;
                   t5:=t-4;
                   t6:=t-5;
                   x1:=t1-1;
                   x2:=0.0;
                   x3:=0.5+cos(-2/3*pi-2/3*pi*t3);
                   x4:=0.0;
                   x5:=-t5;
                   x6:=-1;
                   if(t<1,x1,(if(t<2,x2,(if(t<3,x3,(if(t<4,x4,(if(t<5,x5,x6)))))))))'
    y_formula = 't1:=t;
                   t2:=t-1;
                   t3:=t-2;
                   t4:=t-3;
                   t5:=t-4;
                   t6:=t-5;
                   y1:=-1;
                   y2:=-1+(1-sqrt(3)/2)*t2;
                   y3:=sin(-2/3*pi-2/3*pi*t3);
                   y4:=sqrt(3)/2+(1-sqrt(3)/2)*t4;
                   y5:=1;
                   y6:=1-t6*2;
                   if(t<1,y1,(if(t<2,y2,(if(t<3,y3,(if(t<4,y4,(if(t<5,y5,y6)))))))))'
    section_bounding_t_values = '0 1 2 3 4 5 6'
    constant_names = 'pi'
    constant_expressions = '${fparse pi}'
    nums_segments = '5 2 10 2 5 10'
    is_closed_loop = true
  []
  [xyd]
    type = XYDelaunayGenerator
    boundary = pmg
    desired_area = 0.05
    refine_boundary = false
  []
  [add_bdry]
    type = ParsedGenerateSideset
    input = xyd
    combinatorial_geometry = 'abs((x-0.5)^2+y^2-1.0)<tol'
    constant_names = 'tol'
    constant_expressions = '0.05'
    new_sideset_name = 'circ'
  []
  [ccg]
    type = CircularBoundaryCorrectionGenerator
    input = add_bdry
    input_mesh_circular_boundaries = 'circ'
    custom_circular_tolerance = 1e-8
    move_end_nodes_in_span_direction = true
  []
[]
