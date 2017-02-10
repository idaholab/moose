# A function name that could be interpreted as a ParsedFunction

[Mesh]
  type = GeneratedMesh
  dim = 2
[]

[Functions]
  [./x]
    type = ConstantFunction
  [../]
[]

[Variables]
  [./var]
  [../]
[]

[ICs]
  [./dummy]
    type = FunctionIC
    variable = var
    function = x
  [../]
[]


[Kernels]
  [./diff]
    type = Diffusion
    variable = var
  [../]
[]

[Executioner]
  type = Steady
[]
