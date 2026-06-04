[Mesh]
  [outer_bdy]
    type = ParsedCurveGenerator
    x_formula = 'r*cos(t)'
    y_formula = 'r*sin(t)'
    section_bounding_t_values = '${fparse 0.0} ${fparse pi} ${fparse 2.0*pi}'
    constant_names = 'r'
    constant_expressions = '1.0'
    nums_segments = '8 12'
    is_closed_loop = true
  []
  [hole_1]
    type = PolyLineMeshGenerator
    points = '-0.5 -0.1 0.0
              -0.3 -0.1 0.0
              -0.3 0.1 0.0
              -0.5 0.1 0.0'
    loop = true
    nums_edges_between_points = 2
  []
  [hole_2]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 1
    ny = 2
    xmin = 0.4
    xmax = 0.5
    ymin = -0.1
    ymax = 0.1
  []
  [xyd]
    type = XYDelaunayGenerator
    boundary = outer_bdy
    outer_boundary_layer_thickness = 0.1
    outer_boundary_layer_num = 2
    outer_boundary_layer_bias = 1.5
    holes_boundary_layer_thickness = '0.1 0.15'
    holes_boundary_layer_num = '2 3'
    holes_boundary_layer_bias = '1.0 1.2'
    refine_boundary = false
    stitch_holes = 'false true'
    refine_holes = 'false false'
    holes = 'hole_1 hole_2'
    verify_holes = false
    output_subdomain_id = 1
    output_subdomain_name = 'tri'
  []
[]

[Problem]
  solve = false
[]

[AuxVariables]
  [unity]
    family = MONOMIAL
    order = CONSTANT
    initial_condition = 1.0
  []
[]

[Executioner]
  type = Transient
  num_steps = 1
[]

[Postprocessors]
  [int_unity_hole_2]
    type = ElementIntegralVariablePostprocessor
    variable = unity
    block = 0
  []
  [int_unity_xyd]
    type = ElementIntegralVariablePostprocessor
    variable = unity
    block = 1
  []
[]

[Outputs]
  file_base = 'xydelaunay_bdry_layer'
  csv = true
  execute_on = 'FINAL'
[]
