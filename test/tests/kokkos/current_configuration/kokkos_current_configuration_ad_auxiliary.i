# AD diffusion with a position-dependent AD diffusivity and an AD boundary flux, posed on the current
# configuration of auxiliary displacements, which carry no derivatives. The hand-coded counterparts
# solve the same problem for w, so u and w agree. The Jacobian test adds dependences of the
# diffusivity and of the flux on u through b and BCs/u_flux/v.

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
  [u]
    [InitialCondition]
      type = FunctionIC
      function = 'x * x + 3 * x * y'
    []
  []
  [w]
  []
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
      function = '0.2 * x * x + 0.5 * y'
    []
  []
  [v]
    [InitialCondition]
      type = FunctionIC
      function = '1 + x * y'
    []
  []
[]

[Kernels]
  [u]
    type = KokkosADMatDiffusion
    variable = u
    diffusivity = k
    use_displaced_mesh = true
  []
  [w]
    type = KokkosMatDiffusionTest
    variable = w
    prop_name = k_hand
    use_displaced_mesh = true
  []
[]

[BCs]
  [u_left]
    type = KokkosADDirichletBC
    variable = u
    boundary = left
    value = 0
  []
  [u_flux]
    type = KokkosADCoupledVarNeumannBC
    variable = u
    boundary = 'right top'
    v = v
    use_displaced_mesh = true
  []
  [w_left]
    type = KokkosDirichletBC
    variable = w
    boundary = left
    value = 0
  []
  [w_flux]
    type = KokkosCoupledVarNeumannBC
    variable = w
    boundary = 'right top'
    v = v
    use_displaced_mesh = true
  []
[]

[Materials]
  # k = 2 + 0.5 v^2 + b u + g . x, read by the AD kernel
  [k]
    type = KokkosADQuadraticMaterial
    T = v
    k0 = 2
    a = 0.5
    S = u
    b = 0
    g = '0.3 -0.7 0'
    property_name = k
    use_displaced_mesh = true
  []
  # The same diffusivity without the dependence on u, read by the hand-coded kernel
  [k_hand]
    type = KokkosADQuadraticMaterial
    T = v
    k0 = 2
    a = 0.5
    g = '0.3 -0.7 0'
    property_name = k_hand
    use_displaced_mesh = true
  []
[]

[Postprocessors]
  [u]
    type = KokkosElementIntegralVariablePostprocessor
    variable = u
    use_displaced_mesh = true
  []
  [difference]
    type = ElementL2Difference
    variable = u
    other_variable = w
  []
[]

[Executioner]
  type = Steady
  solve_type = NEWTON
  petsc_options_iname = '-pc_type'
  petsc_options_value = 'lu'
[]

[Outputs]
  csv = true
[]
