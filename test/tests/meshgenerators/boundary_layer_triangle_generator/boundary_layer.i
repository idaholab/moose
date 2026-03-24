[Mesh]
  [outer_bdy]
    type = ParsedCurveGenerator
    x_formula = 'r*cos(t)'
    y_formula = 'r*sin(t)'
    section_bounding_t_values = '0.0 ${fparse pi} ${fparse 2.0*pi}'
    constant_names = 'r'
    constant_expressions = '1.0'
    nums_segments = '16 12'
    is_closed_loop = true
  []
  [boundary_layer]
    type = BoundaryLayerTriangleGenerator
    input = 'outer_bdy'
    thickness = 0.3
    num_layers = 3
    subdomain_id = 1
    subdomain_name = 'boundary_layer'
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
  [int_unity]
    type = ElementIntegralVariablePostprocessor
    variable = unity
  []
[]

[Outputs]
  file_base = 'boundary_layer_outward'
  csv = true
  execute_on = 'FINAL'
[]
