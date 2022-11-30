[Mesh]
  [gen]
    type = CartesianMeshGenerator
    dim = 1
    dx = '1 1 1 1'
    subdomain_id = '1 2 3 4'
  []
[]

[Problem]
  solve = false
[]

[GlobalParams]
  family = MONOMIAL
  order = CONSTANT
[]

[AuxVariables]
  [min_var][]
  [max_var][]
  [sum_var][]
  [average_var][]
  [arr_var]
    components = 4
  []
[]

[AuxKernels]
  [min_aux]
    type = ArrayVarReductionAux
    variable = min_var
    array_variable = arr_var
    value_type = min
  []

  [max_aux]
    type = ArrayVarReductionAux
    variable = max_var
    array_variable = arr_var
    value_type = max
  []

  [sum_aux]
    type = ArrayVarReductionAux
    variable = sum_var
    array_variable = arr_var
    value_type = sum
  []

  [average_aux]
    type = ArrayVarReductionAux
    variable = average_var
    array_variable = arr_var
    value_type = average
  []
[]

[ICs]
  [arr_var_ic_1]
    type = ArrayConstantIC
    variable = arr_var
    value = '1 2 3 4'
    block = 1
  []

  [arr_var_ic_2]
    type = ArrayConstantIC
    variable = arr_var
    value = '1 2 3 -4'
    block = 2
  []

  [arr_var_ic_3]
    type = ArrayConstantIC
    variable = arr_var
    value = '100 0 0 0'
    block = 3
  []

  [arr_var_ic_4]
    type = ArrayConstantIC
    variable = arr_var
    value = '100 2 1 -1000'
    block = 4
  []
[]

[Executioner]
  type = Steady
[]

[Outputs]
  exodus = true
[]
