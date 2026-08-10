[Mesh]
  [outer_bdy]
    type = PolyLineMeshGenerator
    points = '0.0 0.0 0.0
              3.0 0.0 0.0
              3.0 1.0 0.0
              0.0 1.0 0.0'
    loop = true
    nums_edges_between_points = '21 7 21 7'
  []
  [hole_left]
    type = ParsedCurveGenerator
    x_formula = 'x0 + r*cos(t)'
    y_formula = 'y0 + r*sin(t)'
    section_bounding_t_values = '${fparse 0.0} ${fparse pi} ${fparse 2.0*pi}'
    constant_names = 'x0 y0 r'
    constant_expressions = '0.6 0.5 0.22'
    nums_segments = '5 5'
    is_closed_loop = true
  []
  [hole_center]
    type = ParsedCurveGenerator
    x_formula = 'x0 + r*cos(t)'
    y_formula = 'y0 + r*sin(t)'
    section_bounding_t_values = '${fparse 0.0} ${fparse pi} ${fparse 2.0*pi}'
    constant_names = 'x0 y0 r'
    constant_expressions = '1.5 0.5 0.22'
    nums_segments = '5 5'
    is_closed_loop = true
  []
  [hole_right]
    type = ParsedCurveGenerator
    x_formula = 'x0 + r*cos(t)'
    y_formula = 'y0 + r*sin(t)'
    section_bounding_t_values = '${fparse 0.0} ${fparse pi} ${fparse 2.0*pi}'
    constant_names = 'x0 y0 r'
    constant_expressions = '2.4 0.5 0.22'
    nums_segments = '5 5'
    is_closed_loop = true
  []
  [triang]
    type = XYFrontalDelaunayGenerator
    boundary = 'outer_bdy'
    holes = 'hole_left
             hole_center
             hole_right'
    refine_boundary = false
    refine_holes = 'false false false'
    desired_area = 0.01
    metric = LINF
    orientation = CROSS_FIELD
    output_subdomain_name = 'mbb'
  []
  [to_quad]
    type = TriToQuadGenerator
    input = triang
    algorithm = RECOMBINE
    matching = GREEDY
    eta_min = 0.3
    all_quad = true
  []
  [smooth]
    type = SmoothMeshGenerator
    input = to_quad
    algorithm = laplace
  []
[]

[Problem]
  solve = false
[]

[AuxVariables]
  [quality]
    family = MONOMIAL
    order = CONSTANT
  []
[]

[AuxKernels]
  [qa]
    type = ElementQualityAux
    variable = quality
    metric = SHAPE
  []
[]

[Executioner]
  type = Transient
  num_steps = 1
[]

[Postprocessors]
  [area]
    type = VolumePostprocessor
    outputs = csv
  []
  [avg_quality]
    type = ElementAverageValue
    variable = quality
    outputs = csv
  []
  [elem_size]
    type = AverageElementSize
    outputs = csv
  []
[]

[Outputs]
  csv = true
  execute_on = 'FINAL'
[]
