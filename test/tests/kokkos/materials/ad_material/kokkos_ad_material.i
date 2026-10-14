# Integrals of a Kokkos AD material property k = k0 + a T^2 + g . x on the reference and the
# current configuration, compared against the same property computed by a ParsedMaterial on the
# undisplaced and the displaced mesh

[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 4
  ny = 4
  xmin = 1
  xmax = 2
  displacements = 'disp_x disp_y'
[]

[Variables]
  [T]
    [InitialCondition]
      type = FunctionIC
      function = '1 + x * y'
    []
  []
  [disp_x]
    [InitialCondition]
      type = FunctionIC
      function = '0.1 * x * y + 0.2 * x'
    []
  []
  [disp_y]
    [InitialCondition]
      type = FunctionIC
      function = '0.2 * x * x + 0.5 * y'
    []
  []
[]

[Kernels]
  [T]
    type = KokkosDiffusion
    variable = T
  []
  [disp_x]
    type = KokkosDiffusion
    variable = disp_x
  []
  [disp_y]
    type = KokkosDiffusion
    variable = disp_y
  []
[]

[Materials]
  [reference_ad]
    type = KokkosADQuadraticMaterial
    T = T
    k0 = 2
    a = 0.5
    g = '0.3 -0.7 0'
    property_name = k_reference
  []
  [current_ad]
    type = KokkosADQuadraticMaterial
    T = T
    k0 = 2
    a = 0.5
    g = '0.3 -0.7 0'
    property_name = k_current
    use_displaced_mesh = true
  []
  [reference_parsed]
    type = ParsedMaterial
    property_name = k_reference_parsed
    coupled_variables = T
    expression = '2 + 0.5 * T * T + 0.3 * x - 0.7 * y'
    extra_symbols = 'x y'
  []
  [current_parsed]
    type = ParsedMaterial
    property_name = k_current_parsed
    coupled_variables = T
    expression = '2 + 0.5 * T * T + 0.3 * x - 0.7 * y'
    extra_symbols = 'x y'
    use_displaced_mesh = true
  []
[]

[Postprocessors]
  [reference]
    type = KokkosElementIntegralMaterialProperty
    mat_prop = k_reference
    execute_on = INITIAL
  []
  [reference_parsed]
    type = ElementIntegralMaterialProperty
    mat_prop = k_reference_parsed
    execute_on = INITIAL
  []
  [current]
    type = KokkosElementIntegralMaterialProperty
    mat_prop = k_current
    use_displaced_mesh = true
    execute_on = INITIAL
  []
  [current_parsed]
    type = ElementIntegralMaterialProperty
    mat_prop = k_current_parsed
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
