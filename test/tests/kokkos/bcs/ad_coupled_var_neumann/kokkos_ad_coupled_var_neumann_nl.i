[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 20
[]

[Variables]
  [u][]
  [v][]
[]

[Kernels]
  [diff]
    type = KokkosADDiffusion
    variable = u
  []
  [diff_v]
    type = KokkosADDiffusion
    variable = v
  []
[]

[BCs]
  [left]
    type = KokkosADDirichletBC
    variable = u
    boundary = 'left'
    value = 0
  []
  [right]
    type = KokkosADCoupledVarNeumannBC
    variable = u
    boundary = 'right'
    v = v
  []
  [v_left]
    type = KokkosADDirichletBC
    variable = v
    boundary = 'left'
    value = 0
  []
  [v_right]
    type = KokkosADDirichletBC
    variable = v
    boundary = 'right'
    value = 1
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
  residual_and_jacobian_together = true
[]

[Outputs]
  exodus = true
[]
