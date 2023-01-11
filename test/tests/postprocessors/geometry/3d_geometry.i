radius = 0.5
inner_box_length = 2.2
outer_box_length = 3
depth = 0.4
sides = 28
alpha = ${fparse 2 * pi / ${sides}}
perimeter_correction = ${fparse ${alpha} / 2 / sin(alpha / 2)}
area_correction = ${fparse alpha / sin(alpha)}

[Mesh]
  file = 3d.e
  construct_side_list_from_node_list = true
[]

[Problem]
  solve = false
[]

[Executioner]
  type = Steady
[]

[Postprocessors]
  [./circle_side_area]
    type = AreaPostprocessor
    boundary = circle_side
  [../]
  [./inside_side_area]
    type = AreaPostprocessor
    boundary = inside_side
  [../]
  [./outside_side_area]
    type = AreaPostprocessor
    boundary = outside_side
  [../]
  [./circle_volume]
    type = VolumePostprocessor
    block = circle
  [../]
  [./inside_volume]
    type = VolumePostprocessor
    block = inside
  [../]
  [./outside_volume]
    type = VolumePostprocessor
    block = outside
  [../]
  [./total_volume]
    type = VolumePostprocessor
    block = 'circle inside outside'
  [../]

  [./circle_side_area_exact]
    type = FunctionValuePostprocessor
    function = 'circle_side_area_exact'
  [../]
  [./inside_side_area_exact]
    type = FunctionValuePostprocessor
    function = 'inside_side_area_exact'
  [../]
  [./outside_side_area_exact]
    type = FunctionValuePostprocessor
    function = 'outside_side_area_exact'
  [../]
  [./circle_volume_exact]
    type = FunctionValuePostprocessor
    function = 'circle_volume_exact'
  [../]
  [./inside_volume_exact]
    type = FunctionValuePostprocessor
    function = 'inside_volume_exact'
  [../]
  [./outside_volume_exact]
    type = FunctionValuePostprocessor
    function = 'outside_volume_exact'
  [../]
  [./total_volume_exact]
    type = FunctionValuePostprocessor
    function = 'total_volume_exact'
  [../]
[]

[Functions]
  [./circle_side_area_exact]
    type = ParsedFunction
    expression = '2 * pi * ${radius} / ${perimeter_correction} * ${depth}'
  [../]
  [./inside_side_area_exact]
    type = ParsedFunction
    expression = '${inner_box_length} * ${depth} * 4'
  [../]
  [./outside_side_area_exact]
    type = ParsedFunction
    expression = '${outer_box_length} * ${depth} * 4'
  [../]
  [./circle_volume_exact]
    type = ParsedFunction
    expression = 'pi * ${radius}^2 * ${depth} / ${area_correction}'
  [../]
  [./inside_volume_exact]
    type = ParsedFunction
    expression = '${inner_box_length}^2 * ${depth} - pi * ${radius}^2 * ${depth} / ${area_correction}'
  [../]
  [./outside_volume_exact]
    type = ParsedFunction
    expression = '${outer_box_length}^2 * ${depth} - ${inner_box_length}^2 * ${depth}'
  [../]
  [./total_volume_exact]
    type = ParsedFunction
    expression = '${outer_box_length}^2 * ${depth}'
  [../]

[]

[Outputs]
  csv = true
[]
