[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 10
  ny = 10
  nz = 0
  zmin = 0
  zmax = 0
  elem_type = QUAD4
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
    value = 0.0
  []

  [right]
    type = KokkosConvectiveFluxBC
    variable = u
    boundary = 1
    rate = 100
    initial = 10
    final = 20
    duration = 10
  []
[]

[Executioner]
  type = Transient

  solve_type = 'PJFNK'

  start_time = 0.0
  num_steps = 10
  dt = 1.0
[]

[Outputs]
  exodus = true
[]
