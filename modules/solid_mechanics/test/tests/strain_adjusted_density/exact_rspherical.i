x_length = 0.2

[Mesh]
  [mesh]
    type = GeneratedMeshGenerator
    dim = 1
    nx = 3
    xmax = ${x_length}
  []
  coord_type = RSPHERICAL
[]

[Problem]
  solve = false
[]

[AuxVariables]
  [disp_r]
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
    displacements = 'disp_r'
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
    pp_names = 'disp_r'
    expression = 't / (1 + disp_r / ${x_length})^3'
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
