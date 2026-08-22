[Mesh]
  [circle]
    type = ParsedCurveGenerator
    x_formula = 'r*cos(t)'
    y_formula = 'r*sin(t)'
    section_bounding_t_values = '${fparse 0.0} ${fparse pi} ${fparse 2.0*pi}'
    constant_names = 'r'
    constant_expressions = '1.0'
    nums_segments = '16 16'
    is_closed_loop = true
  []
  [triang]
    type = XYFrontalDelaunayGenerator
    boundary = 'circle'
    refine_boundary = false
    add_nodes_per_boundary_segment = 2
    desired_area = 0.02
    output_boundary = 'circumference'
    output_subdomain_name = 'disk'
  []
  [to_quad]
    type = TriToQuadGenerator
    input = triang
    all_quad = true
  []
  [snap]
    type = ParsedCurveNodeSnapGenerator
    input = to_quad
    boundary = 'circumference'
    curve_generator = circle
  []
  [smooth]
    type = SmoothMeshGenerator
    input = snap
    algorithm = laplace
  []
[]

[Problem]
  solve = false
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
[]

[Outputs]
  csv = true
  execute_on = 'FINAL'
[]
