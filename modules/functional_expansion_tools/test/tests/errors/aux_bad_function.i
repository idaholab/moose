[Mesh]
  type = GeneratedMesh
  dim = 1
[]

[AuxVariables]
  [./v]
  [../]
[]

[AuxKernels]
  [./this_could_be_bad]
    type = FunctionSeriesToAux
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
