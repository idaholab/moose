# Integrals over the current configuration computed by Kokkos postprocessors with
# use_displaced_mesh = true are compared against integrals over the displaced mesh, including the
# integral of a position-dependent material property evaluated on the current configuration.

[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 4
  ny = 4
  xmin = 1
  xmax = 2
  displacements = 'disp_x disp_y'
[]

[AuxVariables]
  [disp_x]
    [InitialCondition]
      type = FunctionIC
      function = '0.1 * x * y + 0.2 * x'
    []
  []
  [disp_y]
    [InitialCondition]
      type = FunctionIC
      function = '0.2 * x * x + y'
    []
  []
  [u]
    [InitialCondition]
      type = FunctionIC
      function = 'x + 2 * y'
    []
  []
[]

[Postprocessors]
  [reference_volume]
    type = KokkosElementIntegralVariablePostprocessor
    variable = u
    execute_on = INITIAL
  []
  [current_volume]
    type = KokkosElementIntegralVariablePostprocessor
    variable = u
    use_displaced_mesh = true
    execute_on = INITIAL
  []
  [displaced_volume]
    type = ElementIntegralVariablePostprocessor
    variable = u
    use_displaced_mesh = true
    execute_on = INITIAL
  []
  [reference_property]
    type = KokkosElementIntegralMaterialProperty
    mat_prop = kokkos_xy
    execute_on = INITIAL
  []
  [current_property]
    type = KokkosElementIntegralMaterialProperty
    mat_prop = kokkos_xy_displaced
    use_displaced_mesh = true
    execute_on = INITIAL
  []
  [displaced_property]
    type = ElementIntegralMaterialProperty
    mat_prop = xy_displaced
    use_displaced_mesh = true
    execute_on = INITIAL
  []
  [reference_side]
    type = KokkosSideIntegralVariablePostprocessor
    variable = u
    boundary = 'right top'
    execute_on = INITIAL
  []
  [current_side]
    type = KokkosSideIntegralVariablePostprocessor
    variable = u
    boundary = 'right top'
    use_displaced_mesh = true
    execute_on = INITIAL
  []
  [displaced_side]
    type = SideIntegralVariablePostprocessor
    variable = u
    boundary = 'right top'
    use_displaced_mesh = true
    execute_on = INITIAL
  []
[]

[Materials]
  [reference_position]
    type = KokkosParsedMaterial
    property_name = kokkos_xy
    expression = 'x * y'
    use_xyzt = true
  []
  [current_position]
    type = KokkosParsedMaterial
    property_name = kokkos_xy_displaced
    expression = 'x * y'
    use_xyzt = true
    use_displaced_mesh = true
  []
  [displaced_position]
    type = ParsedMaterial
    property_name = xy_displaced
    expression = 'x * y'
    extra_symbols = 'x y'
    use_displaced_mesh = true
  []
[]

[Problem]
  solve = false
[]

[Executioner]
  type = Steady
[]

[Outputs]
  csv = true
  execute_on = INITIAL
[]
