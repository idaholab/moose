[Mesh]
  [square]
    type = GeneratedMeshGenerator
    nx = 2
    ny = 2
    dim = 2
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
    type = KokkosDiffusion
    variable = u
  []
[]

[BCs]
  [left]
    type = KokkosDirichletBC
    variable = u
    boundary = 3
    value = 0
  []

  [right]
    type = KokkosDirichletBC
    variable = u
    boundary = 1
    value = 1
  []
[]

[Executioner]
  type = Steady
  solve_type = 'NEWTON'
[]

[VectorPostprocessors]
  [boundary_solution]
    type = KokkosBoundarySolutionOutput
    variable = u
    boundary = '0 1 2 3'
  []
[]

[Outputs]
  csv = true
[]
