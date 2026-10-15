# Jacobian of Kokkos AD diffusion and an AD boundary flux whose coefficient r is computed by a chain
# of Kokkos AD materials: k(T, S) on the current configuration, q(k, u) = k^2 + c u + g . x, and
# r(q) = q^2 on the reference configuration. The derivatives of k with respect to T, S, and the
# displacements compose through q and r into the Jacobian of the consumers. The materials are listed
# after their consumers, so that their slots are resolved in dependency order rather than input
# order.

[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 4
    ny = 4
    xmin = 1
    xmax = 2
  []
  displacements = 'disp_x disp_y'
[]

[Variables]
  [T]
    [InitialCondition]
      type = FunctionIC
      function = '1 + 0.5 * x * y'
    []
  []
  [S]
    [InitialCondition]
      type = FunctionIC
      function = '0.5 + 0.2 * (x - y)'
    []
  []
  [u]
    [InitialCondition]
      type = FunctionIC
      function = '0.3 * x * x + 0.2 * x * y'
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
    type = KokkosADMatDiffusion
    variable = T
    diffusivity = r
  []
  [S]
    type = KokkosADMatDiffusion
    variable = S
    diffusivity = q
    use_displaced_mesh = true
  []
  [u]
    type = KokkosADMatDiffusion
    variable = u
    diffusivity = r
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

[BCs]
  [T]
    type = KokkosADMatNeumannBC
    variable = T
    boundary = 'right top'
    property = r
  []
[]

[Materials]
  [r]
    type = KokkosADChainedMaterial
    p = q
    property_name = r
  []
  [q]
    type = KokkosADChainedMaterial
    p = k
    v = u
    c = 0.4
    g = '0.2 -0.1 0'
    property_name = q
  []
  [k]
    type = KokkosADQuadraticMaterial
    T = T
    S = S
    k0 = 1
    a = 0.3
    b = 0.5
    g = '0.3 -0.2 0'
    property_name = k
    use_displaced_mesh = true
  []
[]

[Preconditioning]
  [smp]
    type = SMP
    full = true
  []
[]

[Executioner]
  type = Steady
  solve_type = NEWTON
[]
