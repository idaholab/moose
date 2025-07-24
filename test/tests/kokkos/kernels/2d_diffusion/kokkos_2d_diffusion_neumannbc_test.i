[Mesh]
  [square]
    type = GeneratedMeshGenerator
    nx = 2
    ny = 2
    dim = 2
  []
[]

[Variables]
  active = 'u'

  [u]
    order = FIRST
    family = LAGRANGE
  []
[]

[KokkosKernels]
  active = 'diff'
  [diff]
    type = KokkosDiffusion
    variable = u
  []
[]

[KokkosBCs]
  active = 'left right'
  [left]
    type = KokkosDirichletBC
    variable = u
    boundary = 3
    value = 0
  []
  [right]
    type = KokkosNeumannBC
    variable = u
    boundary = 1
    value = 1
  []
[]

[Executioner]
  type = Steady

  solve_type = 'PJFNK'
[]

[Outputs]
  exodus = true
[]
