[Mesh]
  [outer_bdy]
    type = ParsedCurveGenerator
    x_formula = 'r*cos(t)'
    y_formula = 'r*sin(t)'
    section_bounding_t_values = '${fparse 0.0} ${fparse pi} ${fparse 2.0*pi}'
    constant_names = 'r'
    constant_expressions = '1.0'
    nums_segments = '16 16'
    is_closed_loop = true
  []
  [inner_bdy]
    type = ParsedCurveGenerator
    x_formula = 'r*cos(t)'
    y_formula = 'r*sin(t)'
    section_bounding_t_values = '${fparse 0.0} ${fparse pi} ${fparse 2.0*pi}'
    constant_names = 'r'
    constant_expressions = '0.35'
    nums_segments = '6 6'
    is_closed_loop = true
  []
  [triang]
    type = XYFrontalDelaunayGenerator
    boundary = 'outer_bdy'
    holes = 'inner_bdy'
    hole_boundaries = 'inner'
    refine_boundary = false
    refine_holes = 'false'
    desired_area = 0.02
    output_subdomain_name = 'triangles'
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
  # The hole boundary is a 12-sided polygon whose vertices the front never moves, so the
  # perimeter of the 'inner' sideset is fixed by the input polygon no matter how the interior
  # is triangulated; it also fails if the hole boundary name is not carried onto the sideset
  [hole_perimeter]
    type = AreaPostprocessor
    boundary = 'inner'
    outputs = csv
  []
[]

[Outputs]
  csv = true
  execute_on = 'FINAL'
[]
