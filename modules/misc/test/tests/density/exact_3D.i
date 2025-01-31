[Mesh]
  [mesh]
    type = PatchMeshGenerator
    dim = 3
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
  [strain_free_density]
    type = ParsedMaterial
    property_name = strain_free_density
    expression = t
    extra_symbols = t
    outputs = all
  []
  [density]
    type = Density
    strain_free_density = strain_free_density
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
  [strain_free_density]
    type = ElementAverageValue
    variable = strain_free_density
  []
  [density]
    type = ElementAverageValue
    variable = density
  []
  [density_exact]
    type = ParsedPostprocessor
    pp_names = 'disp_x disp_y disp_z'
    expression = '1 / (1 + disp_x) / (1 + disp_y) / (1 + disp_z)'
  []
[]

[Outputs]
  csv = true
[]
