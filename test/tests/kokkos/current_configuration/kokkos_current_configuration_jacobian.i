# Jacobian of a diffusion term, a volumetric source, a position-dependent source, and a flux through
# the boundary normal posed on the current configuration, with respect to the solution and to
# nonlinear displacements

[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 3
  ny = 3
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
  [diffusion]
    type = KokkosDiffusion
    variable = u
    use_displaced_mesh = true
  []
  [source]
    type = KokkosBodyForce
    variable = u
    value = 10
    use_displaced_mesh = true
  []
  [position_source]
    type = KokkosXYBodyForce
    variable = u
    value = 3
    use_displaced_mesh = true
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
  [flux]
    type = KokkosDirectionalNeumannBC
    variable = u
    boundary = 'right top'
    vector_value = '1 2 0'
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
