# Stored derivatives of a Kokkos AD material property k = k0 + a T^2 + b S + g . x whose declaration
# lists its inputs. Listing S ahead of T stores dk/dS at index 0 and dk/dT at index 1, followed on
# the current configuration by the displacement derivatives, so that each stored derivative matches
# its analytic value: dk/dS = b, dk/dT = 2 a T, and dk/d(disp) = g.

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
  [S]
    [InitialCondition]
      type = FunctionIC
      function = 'x - y'
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
  [S]
    type = KokkosDiffusion
    variable = S
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
  [k]
    type = KokkosADQuadraticMaterial
    T = T
    S = S
    k0 = 2
    a = 0.5
    b = 1.5
    g = '0.3 -0.7 0'
    listed_inputs = 'S T'
    property_name = k
    use_displaced_mesh = true
  []
  [dk_dS]
    type = KokkosADDerivativeSlotMaterial
    ad_property = k
    slot = 0
    property_name = dk_dS
  []
  [dk_dT]
    type = KokkosADDerivativeSlotMaterial
    ad_property = k
    slot = 1
    property_name = dk_dT
  []
  [dk_ddisp_x]
    type = KokkosADDerivativeSlotMaterial
    ad_property = k
    slot = 2
    property_name = dk_ddisp_x
  []
  [dk_ddisp_y]
    type = KokkosADDerivativeSlotMaterial
    ad_property = k
    slot = 6
    property_name = dk_ddisp_y
  []
[]

[Postprocessors]
  [dk_dS]
    type = KokkosElementIntegralMaterialProperty
    mat_prop = dk_dS
    execute_on = INITIAL
  []
  [dk_dT]
    type = KokkosElementIntegralMaterialProperty
    mat_prop = dk_dT
    execute_on = INITIAL
  []
  [dk_ddisp_x]
    type = KokkosElementIntegralMaterialProperty
    mat_prop = dk_ddisp_x
    execute_on = INITIAL
  []
  [dk_ddisp_y]
    type = KokkosElementIntegralMaterialProperty
    mat_prop = dk_ddisp_y
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
