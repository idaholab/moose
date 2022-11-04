[Mesh]
  type = FileMesh
  file = two_pipe.e
[]

[Problem]
  kernel_coverage_check = false
[]

[Variables]
  [u]
  []
[]


[AuxVariables]
  [./var]
    order = CONSTANT
    family = MONOMIAL
    block = p1
  [../]
[]

[ICs]
  [./var]
    type = FunctionIC
    variable = var
    function = setvar
    block = p1
  [../]
[]

[Functions]
  [./setvar]
    type = ParsedFunction
    expression = '1 + z * z'
  [../]
[]

[UserObjects]
  [./sub_app_uo]
    type = LayeredAverage
    direction = z
    variable = var
    num_layers = 10
    execute_on = TIMESTEP_END
    block = p1
  [../]
[]

[Executioner]
  type = Transient
[]
