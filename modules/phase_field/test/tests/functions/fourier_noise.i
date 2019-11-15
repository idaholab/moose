[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 20
  ny = 20
[]

[Variables]
  [./c]
  [../]
[]

[Functions]
  [./fn]
    type = FourierNoise
    lambda = 0.2
  [../]
[]

[ICs]
  [./c]
    type = FunctionIC
    variable = c
    function = fn
  [../]
[]

[Problem]
  solve = false
[]

[Executioner]
  type = Steady
[]

[Outputs]
  exodus = true
[]
