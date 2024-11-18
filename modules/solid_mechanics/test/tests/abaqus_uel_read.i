[Mesh]
  type = AbaqusUELMesh
  file = CUBE_UEL.inp
[]

[Variables/AddUELVariables]
[]

[ICs]
  [var_1]
    type = ConstantIC
    value = 1
    variable = var_1
  []
  [var_2]
    type = ConstantIC
    value = 2
    variable = var_2
  []
  [var_4]
    type = ConstantIC
    value = 4
    variable = var_4
  []
  [var_8]
    type = ConstantIC
    value = 8
    variable = var_8
  []
[]

[Problem]
  solve = false
  kernel_coverage_check = false
[]

[Executioner]
  type = Steady
[]

[Outputs]
  exodus = true
[]
