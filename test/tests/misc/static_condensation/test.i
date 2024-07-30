[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 10
  ny = 10
  second_order = true
[]

[Variables]
  [u]
    order = SECOND
  []
[]

[Kernels]
  [diff]
    type = Diffusion
    variable = u
  []
[]

[BCs]
  [left]
    type = PenaltyDirichletBC
    variable = u
    boundary = left
    value = 0
    penalty = 1e8
  []
  [right]
    type = PenaltyDirichletBC
    variable = u
    boundary = right
    value = 1
    penalty = 1e8
  []
[]

[Executioner]
  type = Steady
  solve_type = 'PJFNK'
[]

[Outputs]
  exodus = true
[]
