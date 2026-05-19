[Mesh]
  type = GeneratedMesh
  nx = 5
  ny = 5
  dim = 2
[]

[Variables]
  [u]
  []
  [v]
  []
[]

[Kernels]
  [time_u]
    type = KokkosADTimeDerivative
    variable = u
  []
  [fn_u]
    type = KokkosBodyForce
    variable = u
  []
  [time_v]
    type = KokkosADCoupledTimeDerivative
    variable = v
    v = u
  []
  [diff_v]
    type = KokkosADDiffusion
    variable = v
  []
[]

[BCs]
  [left]
    type = KokkosADDirichletBC
    variable = v
    boundary = 'left'
    value = 0
  []
  [right]
    type = KokkosADDirichletBC
    variable = v
    boundary = 'right'
    value = 1
  []
[]

[Executioner]
  type = Transient
  num_steps = 1
  end_time = 10
  solve_type = 'NEWTON'
  residual_and_jacobian_together = true
[]

[Outputs]
  exodus = true
[]
