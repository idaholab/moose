[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 2
  ny = 2
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
    boundary = 0
    value = 2.0
  []
  [left]
    type = KokkosDirichletBC
    variable = u
    boundary = 3
    value = 0.0
  []

  [top]
    type = KokkosDirectionalNeumannBC
    variable = u
    vector_value = '1 1 0'
    boundary = 2
  []

  [right]
    type = KokkosDirectionalNeumannBC
    variable = u
    vector_value = '-1 -1 0'
    boundary = 1
  []
[]

[Executioner]
  type = Steady
  solve_type = 'PJFNK'
[]

[Outputs]
  exodus = true
[]
