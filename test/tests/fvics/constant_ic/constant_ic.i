[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    nx = 2
    ny = 2
    dim = 2
  []
[]

[Variables]
  [u]
    type = MooseVariableFVReal
    [FVInitialCondition]
      type = FVConstantIC
      value = 6.2
    []
  []
  [v]
    type = MooseVariableFVReal
    initial_condition = 3.1
  []
  [w]
    type = MooseVariableFVReal
  []
[]

[AuxVariables]
  [u_aux]
    type = MooseVariableFVReal
    [FVInitialCondition]
      type = FVConstantIC
      value = 1.65
    []
  []
[]

[FVICs]
  [cu]
    type = FVConstantIC
    variable = w
    value = 9.3
  []
[]

[Executioner]
  type = Steady
[]

[Problem]
  solve = false
[]

[Outputs]
  exodus = true
[]
