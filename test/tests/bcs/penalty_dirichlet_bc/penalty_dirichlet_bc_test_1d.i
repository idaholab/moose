[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 1
    nx = 10
  []
[]

[Variables]
  [u]
    order = FIRST
    family = LAGRANGE
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
    penalty = 1e9
#    type = DirichletBC
    variable = u
    boundary = 'left'
    value = 0
  []

  [right]
    type = PenaltyDirichletBC
    penalty = 1e9
#    type = DirichletBC
    variable = u
    boundary = 'right'
    value = 1
  []
[]

[Executioner]
  type = Steady
[]

[Outputs]
  exodus = true
[]
