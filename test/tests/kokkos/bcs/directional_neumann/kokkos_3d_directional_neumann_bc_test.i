[Mesh]
  type = GeneratedMesh
  dim = 3
  nx = 2
  ny = 2
  nz = 2
[]

[Variables]
  [u]
  []
[]

[Kernels]
  [diff]
    type = KokkosDiffusion
    variable = u
  []
[]

[BCs]
  [bottom]
    type = KokkosDirichletBC
    variable = u
    boundary = 1
    value = 2.0
  []

  [left]
    type = KokkosDirichletBC
    variable = u
    boundary = 4
    value = 0.0
  []

  [front]
    type = KokkosDirichletBC
    variable = u
    boundary = 5
    value = 4.0
  []

  [top]
    type = KokkosDirectionalNeumannBC
    variable = u
    vector_value = '-1 1 1'
    boundary = 3
  []

  [right]
    type = KokkosDirectionalNeumannBC
    variable = u
    vector_value = '1 -1 1'
    boundary = 2
  []

  [back]
    type = KokkosDirectionalNeumannBC
    variable = u
    vector_value = '1 1 -1'
    boundary = 0
  []
[]

[Executioner]
  type = Steady
  solve_type = 'PJFNK'
[]

[Outputs]
  exodus = true
[]
