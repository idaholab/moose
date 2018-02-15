[Mesh]
  type = GeneratedMesh
  dim = 1
[]

[Variables]
  [./v]
  [../]
[]

[BCs]
  [./this_could_be_bad]
    type = FXFluxBC
    boundary = right
    function = const
    variable = v
  [../]
[]

[Functions]
  [./const]
    type = ConstantFunction
    value = -1
  [../]
[]

[Executioner]
  type = Steady
[]

[Problem]
  solve = false
[]
