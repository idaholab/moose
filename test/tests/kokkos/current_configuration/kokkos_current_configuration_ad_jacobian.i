# Jacobian of AD diffusion, a nonlinear AD kernel coupling a second variable, and an AD boundary
# flux coupled to a displacement, all posed on the current configuration, with respect to the
# solution and to nonlinear displacements

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
  [v]
    [InitialCondition]
      type = FunctionIC
      function = '1 + x * y * y'
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
    type = KokkosADDiffusion
    variable = u
    use_displaced_mesh = true
  []
  [nonlinear]
    type = KokkosADNonlinearCoupledTest
    variable = v
    v = u
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
    type = KokkosADCoupledVarNeumannBC
    variable = u
    boundary = 'right top'
    v = disp_y
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
