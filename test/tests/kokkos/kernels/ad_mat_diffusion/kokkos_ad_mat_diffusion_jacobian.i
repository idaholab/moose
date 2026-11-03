# Jacobian of Kokkos AD diffusion with a diffusivity k = k0 + a T^2 + g . x from a Kokkos AD
# material. The material on the current configuration carries derivatives with respect to T and to
# the displacements, which reach the Jacobian of a kernel on the current configuration and of a
# kernel on the reference configuration alike. A second material, restricted to one block, depends
# on a second variable c on that block. Boundary fluxes read the face values of the properties on
# both configurations.

[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 4
    ny = 4
    xmin = 1
    xmax = 2
  []
  [block]
    type = SubdomainBoundingBoxGenerator
    input = gen
    block_id = 1
    bottom_left = '1.5 0 0'
    top_right = '2 1 0'
  []
  displacements = 'disp_x disp_y'
[]

[Variables]
  [T]
    [InitialCondition]
      type = FunctionIC
      function = '1 + x * y'
    []
  []
  [u]
    [InitialCondition]
      type = FunctionIC
      function = 'x * x + 3 * x * y'
    []
  []
  [c]
    [InitialCondition]
      type = FunctionIC
      function = '2 + x - y'
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
  [current]
    type = KokkosADMatDiffusion
    variable = T
    diffusivity = k
    use_displaced_mesh = true
  []
  [reference]
    type = KokkosADMatDiffusion
    variable = u
    diffusivity = k
  []
  [c]
    type = KokkosADMatDiffusion
    variable = c
    diffusivity = kc
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
  [current]
    type = KokkosADMatNeumannBC
    variable = T
    boundary = 'right top'
    property = k
    use_displaced_mesh = true
  []
  [reference]
    type = KokkosADMatNeumannBC
    variable = u
    boundary = 'right top'
    property = k
  []
  [c]
    type = KokkosADMatNeumannBC
    variable = c
    boundary = 'right top'
    property = kc
  []
[]

[Materials]
  [k]
    type = KokkosADQuadraticMaterial
    T = T
    k0 = 2
    a = 0.5
    g = '0.3 -0.7 0'
    property_name = k
    use_displaced_mesh = true
  []
  [kc_0]
    type = KokkosADQuadraticMaterial
    block = 0
    T = T
    k0 = 1
    a = 0.2
    property_name = kc
  []
  [kc_1]
    type = KokkosADQuadraticMaterial
    block = 1
    T = c
    k0 = 3
    a = 0.4
    g = '0.5 0.2 0'
    property_name = kc
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
