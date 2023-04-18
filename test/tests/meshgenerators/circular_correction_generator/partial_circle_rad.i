[Mesh]
  [pmg]
    type = ParsedCurveGenerator
    x_formula = 't1:=t;
                 t2:=t-1;
                 t3:=t-2;
                 t4:=t-3;
                 t5:=t-4;
                 x1:=t1-1;
                 x2:=0.0;
                 x3:=0.5*cos(-pi/2-pi*t3/2);
                 x4:=-0.5-0.5*t4;
                 x5:=-1;
                 if(t<1,x1,(if(t<2,x2,(if(t<3,x3,(if(t<4,x4,x5)))))))'
    y_formula = 't1:=t;
                 t2:=t-1;
                 t3:=t-2;
                 t4:=t-3;
                 t5:=t-4;
                 y1:=-1;
                 y2:=-1+0.5*t2;
                 y3:=0.5*sin(-pi/2-pi*t3/2);
                 y4:=0;
                 y5:=-t5;
                 if(t<1,y1,(if(t<2,y2,(if(t<3,y3,(if(t<4,y4,y5)))))))'
    section_bounding_t_values = '0 1 2 3 4 5'
    constant_names = 'pi'
    constant_expressions = '${fparse pi}'
    nums_segments = '10 5 10 5 10'
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
    combinatorial_geometry = 'abs(x^2+y^2-0.25)<tol'
    constant_names = 'tol'
    constant_expressions = '0.01'
    new_sideset_name = 'circ'
  []
  [ccg]
    type = CircularBoundaryCorrectionGenerator
    input = add_bdry
    input_mesh_circular_boundaries = 'circ'
  []
[]
