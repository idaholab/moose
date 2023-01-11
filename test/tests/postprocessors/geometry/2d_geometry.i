radius = 0.5
inner_box_length = 2.2
outer_box_length = 3
sides = 16
alpha = ${fparse 2 * pi / ${sides}}
perimeter_correction = ${fparse alpha / 2 / sin(alpha / 2)}
area_correction = ${fparse alpha / sin(alpha)}

[Mesh]
  file = 2d.e
  construct_side_list_from_node_list = true
[]

[Variables]
  [./u]
    initial_condition = 1
    block = circle
  [../]
  [./v]
    initial_condition = 2
    block = 'inside outside'
  [../]
[]

[Kernels]
  [./diff_u]
    type = Diffusion
    variable = u
  [../]
  [./diff_v]
    type = Diffusion
    variable = v
  [../]
[]

[BCs]
  [./circle]
    type = DirichletBC
    variable = u
    boundary = circle_side_wrt_inside
    value = 2
  [../]
  [./inner]
    type = DirichletBC
    variable = v
    boundary = circle_side_wrt_circle
    value = 4
  [../]
  [./outer]
    type = DirichletBC
    variable = v
    boundary = outside_side
    value = 6
  [../]
[]


[Executioner]
  type = Steady
[]

[Postprocessors]
  [./u_avg]
    type = ElementAverageValue
    variable = u
    block = circle
  [../]
  [./v_avg]
    type = ElementAverageValue
    variable = v
    block = 'inside outside'
  [../]
  [./circle_perimeter_wrt_circle]
    type = AreaPostprocessor
    boundary = circle_side_wrt_circle
  [../]
  [./circle_perimeter_wrt_inside]
    type = AreaPostprocessor
    boundary = circle_side_wrt_inside
  [../]
  [./inside_perimeter_wrt_inside]
    type = AreaPostprocessor
    boundary = inside_side_wrt_inside
  [../]
  [./inside_perimeter_wrt_outside]
    type = AreaPostprocessor
    boundary = inside_side_wrt_outside
  [../]
  [./outside_perimeter]
    type = AreaPostprocessor
    boundary = outside_side
  [../]
  [./circle_area]
    type = VolumePostprocessor
    block = circle
  [../]
  [./inside_area]
    type = VolumePostprocessor
    block = inside
  [../]
  [./outside_area]
    type = VolumePostprocessor
    block = outside
  [../]
  [./total_area]
    type = VolumePostprocessor
    block = 'circle inside outside'
  [../]

  [./circle_perimeter_exact]
    type = FunctionValuePostprocessor
    function = 'circle_perimeter_exact'
  [../]
  [./inside_perimeter_exact]
    type = FunctionValuePostprocessor
    function = 'inside_perimeter_exact'
  [../]
  [./outside_perimeter_exact]
    type = FunctionValuePostprocessor
    function = 'outside_perimeter_exact'
  [../]
  [./circle_area_exact]
    type = FunctionValuePostprocessor
    function = 'circle_area_exact'
  [../]
  [./inside_area_exact]
    type = FunctionValuePostprocessor
    function = 'inside_area_exact'
  [../]
  [./outside_area_exact]
    type = FunctionValuePostprocessor
    function = 'outside_area_exact'
  [../]
  [./total_area_exact]
    type = FunctionValuePostprocessor
    function = 'total_area_exact'
  [../]
[]

[Functions]
  [./circle_perimeter_exact]
    type = ParsedFunction
    expression = '2 * pi * ${radius} / ${perimeter_correction}'
  [../]
  [./inside_perimeter_exact]
    type = ParsedFunction
    expression = '${inner_box_length} * 4'
  [../]
  [./outside_perimeter_exact]
    type = ParsedFunction
    expression = '${outer_box_length} * 4'
  [../]
  [./circle_area_exact]
    type = ParsedFunction
    expression = 'pi * ${radius}^2 / ${area_correction}'
  [../]
  [./inside_area_exact]
    type = ParsedFunction
    expression = '${inner_box_length}^2 - pi * ${radius}^2 / ${area_correction}'
  [../]
  [./outside_area_exact]
    type = ParsedFunction
    expression = '${outer_box_length}^2 - ${inner_box_length}^2'
  [../]
  [./total_area_exact]
    type = ParsedFunction
    expression = '${outer_box_length}^2'
  [../]

[]

[Outputs]
  csv = true
[]
