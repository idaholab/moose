[Mesh]
  [cir1]
    type = ParsedCurveGenerator
    x_formula = 'r*cos(t*2*pi)'
    y_formula = 'r*sin(t*2*pi)'
    section_bounding_t_values = '0 1'
    constant_names = 'pi r'
    constant_expressions = '${fparse pi} 1.0'
    nums_segments = '10'
    is_closed_loop = true
  []
  [cir2]
    type = ParsedCurveGenerator
    x_formula = 'r*cos(t*2*pi)'
    y_formula = 'r*sin(t*2*pi)+0.5'
    section_bounding_t_values = '0 1'
    constant_names = 'pi r'
    constant_expressions = '${fparse pi} 2.0'
    nums_segments = '15'
    is_closed_loop = true
  []
  [xyd1]
    type = XYDelaunayGenerator
    boundary = cir1
    desired_area = 0.5
    refine_boundary = false
    # output_boundary = '10'
    output_subdomain_name = '10'
  []
  [xyd2]
    type = XYDelaunayGenerator
    boundary = cir2
    holes = xyd1
    refine_holes = 'false'
    desired_area = 0.5
    refine_boundary = false
    output_subdomain_name = '20'
    hole_boundaries = '10'
  []
  [ccg]
    type = CircularBoundaryCorrectionGenerator
    input = xyd2
    input_mesh_circular_boundaries = '0 10'
    custom_circular_tolerance = 1e-8
  []
[]
