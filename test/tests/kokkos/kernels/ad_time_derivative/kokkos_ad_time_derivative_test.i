[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 10
  ny = 10
[]

[Variables]
  [u]
  []
[]

[Kernels]
  [td]
    type = KokkosADTimeDerivative
    variable = u
  []
  [diff]
    type = KokkosADDiffusion
    variable = u
  []
[]

[BCs]
  [left]
    type = KokkosADDirichletBC
    variable = u
    boundary = left
    value = 0
  []
  [right]
    type = KokkosADDirichletBC
    variable = u
    boundary = right
    value = 1
  []
[]

[Executioner]
  type = Transient
  num_steps = 5
  dt = 1e-2
  solve_type = NEWTON
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
  residual_and_jacobian_together = true
[]

[Outputs]
  exodus = true
[]
