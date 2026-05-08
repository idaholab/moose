!include 2d-dam-break-benchmark.i

[Postprocessors]
  [paper_front_position_x_raw]
    type = SubcellInterfacialPosition
    volume_fraction = alpha
    direction = x
    extremum_type = max
    threshold = 0.5
    secondary_min = 0
    secondary_max = '${fparse 2.5 * domain_dims_y / 50.0}'
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [paper_top_height_y_raw]
    type = SubcellInterfacialPosition
    volume_fraction = alpha
    direction = y
    extremum_type = max
    threshold = 0.5
    secondary_min = 0
    secondary_max = '${fparse 2.5 * domain_dims_x / 200.0}'
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [paper_front_Z]
    type = ParsedPostprocessor
    expression = '2.0 * x / ${initial_length} - 1.0'
    pp_names = 'paper_front_position_x_raw'
    pp_symbols = 'x'
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [paper_top_H]
    type = ParsedPostprocessor
    expression = 'h / ${initial_length}'
    pp_names = 'paper_top_height_y_raw'
    pp_symbols = 'h'
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [paper_T]
    type = ParsedPostprocessor
    expression = '2.0 * t * sqrt(${g} / ${initial_length})'
    use_t = true
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [paper_tau]
    type = ParsedPostprocessor
    expression = 't * sqrt(2.0 * ${g} / ${initial_length})'
    use_t = true
    execute_on = 'INITIAL TIMESTEP_END'
  []
[]

[Outputs]
  file_base = '2d-dam-break-benchmark-paper-comparison'
[]
