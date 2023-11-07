[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 2
    xmin = 0
    xmax = 1
    nx = 2
    ymin = -2
    ymax = 3
    ny = 3
  []
  allow_renumbering = false
[]

[Problem]
  kernel_coverage_check = false
  solve = false
[]

[Variables]
  [u]
    type = MooseVariableFVReal
  []
  [v]
    type = MooseVariableFVReal
  []
  [w]
  []
[]

[AuxVariables]
  [aux]
    type = MooseVariableFVReal
  []
[]

[VectorPostprocessors]
  [elem_info]
    type = TestElemInfo
    vars = 'u v w aux'
  []
[]

[Executioner]
  type = Steady
[]

[Outputs]
  csv = true
[]
