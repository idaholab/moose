[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 1
    nx = 5
  []
[]

[Variables]
  [u]
  []
  [v1]
  []
  [v2]
  []
[]

[BCs]
  [left]
    type = CoupledDirichletBC
    boundary = left
    variable = u
    v = 'v1 v2'
    value = 0
  []
[]

[Problem]
  kernel_coverage_check = false
[]

[Executioner]
  type = Steady
[]
