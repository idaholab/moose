x_length = 0.2
y_length = 0.1
z_length = 0.3

[Mesh]
  [mesh]
    type = ExamplePatchMeshGenerator
    dim = 3
    x_length = ${x_length}
    y_length = ${y_length}
    z_length = ${z_length}
  []
[]

[Problem]
  solve = false
[]

[AuxVariables]
  [disp_x]
    initial_condition = 0
  []
  [disp_y]
    initial_condition = 0
  []
  [disp_z]
    initial_condition = 0
  []
[]

[AuxKernels]
  [disp_x_aux]
    type = ParsedAux
    variable = disp_x
    expression = 't * 1e-2 * x'
    use_xyzt = true
  []
  [disp_y_aux]
    type = ParsedAux
    variable = disp_y
    expression = 't * 2e-2 * y'
    use_xyzt = true
  []
  [disp_z_aux]
    type = ParsedAux
    variable = disp_z
    expression = 't * 3e-2 * z'
    use_xyzt = true
  []
[]

[Materials]
  [some_density]
    type = ParsedMaterial
    property_name = some_density
    expression = t
    extra_symbols = t
    outputs = all
  []
  [density]
    type = StrainAdjustedDensity
    strain_free_density = some_density
    displacements = 'disp_x disp_y disp_z'
    outputs = all
  []
[]

[Executioner]
  type = Transient
  num_steps = 10
[]

[Postprocessors]
  [disp_x]
    type = SideAverageValue
    variable = disp_x
    boundary = right
  []
  [disp_y]
    type = SideAverageValue
    variable = disp_y
    boundary = top
  []
  [disp_z]
    type = SideAverageValue
    variable = disp_z
    boundary = front
  []
  [some_density]
    type = ElementAverageValue
    variable = some_density
  []
  [density]
    type = ElementAverageValue
    variable = density
  []
  [density_exact]
    type = ParsedPostprocessor
    pp_names = 'disp_x disp_y disp_z'
    expression = 't / (1 + disp_x / ${x_length}) / (1 + disp_y / ${y_length}) / (1 + disp_z / ${z_length})'
    use_t = true
    outputs = console
  []
  [density_diff]
    type = ParsedPostprocessor
    expression = '(density_exact - density) / density_exact'
    pp_names = 'density density_exact'
    outputs = console
  []
  [density_diff_max]
    type = TimeExtremeValue
    postprocessor = density_diff
    value_type = ABS_MAX
  []
[]

[Outputs]
  csv = true
[]
