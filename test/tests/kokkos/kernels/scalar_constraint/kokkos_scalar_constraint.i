[Mesh]
  type = GeneratedMesh
  dim = 2
  xmin = 0
  xmax = 1
  ymin = 0
  ymax = 1
  nx = 3
  ny = 3
  elem_type = QUAD4
[]

[Variables]
  [u]
    family = LAGRANGE
    order = FIRST
  []
  [alpha]
    family = SCALAR
    order = FIRST
    initial_condition = 1
  []
[]

[Kernels]
  [diff]
    type = KokkosDiffusion
    variable = u
  []
  [scalar]
    type = KokkosScalarVarKernel
    variable = u
    alpha = alpha
  []
[]

[ScalarKernels]
  [alpha_ced]
    type = AlphaCED
    variable = alpha
    value = 10
  []
[]

[BCs]
  [left]
    type = KokkosScalarVarBC
    variable = u
    boundary = '3'
    alpha = alpha
  []
  [right]
    type = KokkosDirichletBC
    variable = u
    boundary = '1'
    value = 0
  []
[]

[Preconditioning]
  [pc]
    type = SMP
    full = true
    solve_type = 'PJFNK'
  []
[]

[Executioner]
  type = Steady
[]

[Outputs]
  exodus = true
  hide = alpha
[]
