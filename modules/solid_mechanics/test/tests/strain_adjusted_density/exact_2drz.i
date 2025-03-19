x_length = 0.2
y_length = 0.1

[Mesh]
  [mesh]
    type = ExamplePatchMeshGenerator
    dim = 2
    x_length = ${x_length}
    y_length = ${y_length}
  []
  coord_type = 'RZ'
[]

[Problem]
  solve = false
[]

[AuxVariables]
  [disp_r]
    initial_condition = 0
  []
  [disp_z]
    initial_condition = 0
  []
[]

[AuxKernels]
  [disp_r_aux]
    type = ParsedAux
    variable = disp_r
    expression = 't * 1e-2 * x'
    use_xyzt = true
  []
  [disp_z_aux]
    type = ParsedAux
    variable = disp_z
    expression = 't * 2e-2 * y'
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
    displacements = 'disp_r disp_z'
    outputs = all
  []
[]

[Executioner]
  type = Transient
  num_steps = 10
[]

[Postprocessors]
  [disp_r]
    type = SideAverageValue
    variable = disp_r
    boundary = right
  []
  [disp_z]
    type = SideAverageValue
    variable = disp_z
    boundary = top
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
    pp_names = 'disp_r disp_z'
    expression = 't / (1 + disp_r / ${x_length})^2 / (1 + disp_z / ${y_length})'
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
