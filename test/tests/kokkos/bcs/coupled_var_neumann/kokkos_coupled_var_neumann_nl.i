[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 20
[]

[Variables]
  [u][]
  [v][]
[]

[KokkosKernels]
  [diff]
    type = KokkosDiffusion
    variable = u
  []
  [diff_v]
    type = KokkosDiffusion
    variable = v
  []
[]

[KokkosBCs]
  [left]
    type = KokkosDirichletBC
    variable = u
    boundary = 'left'
    value = 0
  []
  [right]
    type = KokkosCoupledVarNeumannBC
    variable = u
    boundary = 'right'
    v = v
  []
  [v_left]
    type = KokkosDirichletBC
    variable = v
    boundary = 'left'
    value = 0
  []
  [v_right]
    type = KokkosDirichletBC
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
[]

[Outputs]
  exodus = true
[]
