[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 2
  ny = 2
  displacements = 'disp_x disp_y'
[]

[AuxVariables]
  [disp_x]
    [InitialCondition]
      type = FunctionIC
      function = x
    []
  []
  [disp_y]
    [InitialCondition]
      type = FunctionIC
      function = '2*y'
    []
  []
  [one]
    initial_condition = 1
  []
  [reference_coordinate]
  []
  [displaced_coordinate]
  []
[]

[AuxKernels]
  [reference_coordinate]
    type = KokkosParsedAux
    variable = reference_coordinate
    expression = 'x+y'
    use_xyzt = true
    execute_on = INITIAL
  []
  [displaced_coordinate]
    type = KokkosParsedAux
    variable = displaced_coordinate
    expression = 'x+y'
    use_xyzt = true
    use_displaced_mesh = true
    execute_on = INITIAL
  []
[]

[Materials]
  [reference_coordinate]
    type = KokkosParsedMaterial
    property_name = reference_coordinate
    expression = 'x+y'
    use_xyzt = true
  []
  [displaced_coordinate]
    type = KokkosParsedMaterial
    property_name = displaced_coordinate
    expression = 'x+y'
    use_xyzt = true
    use_displaced_mesh = true
  []
[]

[Postprocessors]
  [reference_volume]
    type = KokkosElementIntegralVariablePostprocessor
    variable = one
    execute_on = INITIAL
  []
  [displaced_volume]
    type = KokkosElementIntegralVariablePostprocessor
    variable = one
    use_displaced_mesh = true
    execute_on = INITIAL
  []
  [reference_aux_integral]
    type = KokkosElementIntegralVariablePostprocessor
    variable = reference_coordinate
    execute_on = INITIAL
  []
  [displaced_aux_integral]
    type = KokkosElementIntegralVariablePostprocessor
    variable = displaced_coordinate
    use_displaced_mesh = true
    execute_on = INITIAL
  []
  [reference_material_integral]
    type = KokkosElementIntegralMaterialProperty
    mat_prop = reference_coordinate
    execute_on = INITIAL
  []
  [displaced_material_integral]
    type = KokkosElementIntegralMaterialProperty
    mat_prop = displaced_coordinate
    use_displaced_mesh = true
    execute_on = INITIAL
  []
  [reference_side]
    type = KokkosSideIntegralVariablePostprocessor
    variable = one
    boundary = right
    execute_on = INITIAL
  []
  [displaced_side]
    type = KokkosSideIntegralVariablePostprocessor
    variable = one
    boundary = right
    use_displaced_mesh = true
    execute_on = INITIAL
  []
[]

[Problem]
  solve = false
  kernel_coverage_check = false
[]

[Executioner]
  type = Steady
[]

[Outputs]
  csv = true
[]
