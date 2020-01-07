[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 100
  ny = 100
[]

[Variables]
  [./c]
  [../]
[]

[Functions]
  [./fn]
    type = FourierNoise
    lambda = 0.1
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
