# Integrals over the current configuration computed by Kokkos postprocessors that couple the
# displacements are compared against integrals over the displaced mesh.

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
    displacements = 'disp_x disp_y'
    execute_on = INITIAL
  []
  [displaced_volume]
    type = ElementIntegralVariablePostprocessor
    variable = u
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
    displacements = 'disp_x disp_y'
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
