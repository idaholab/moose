[Mesh]
  type = GeneratedMesh
  nx = 5
  ny = 5
  nz = 5
  dim = 3
[]

[Problem]
  solve = false 
[]

[AuxVariables]
  [primary_lagrange_vector]
    family = LAGRANGE_VEC
    order = FIRST
  []

  [secondary_lagrange_vector]
    family = LAGRANGE_VEC
    order = FIRST
  []

  [tertiary_lagrange_vector]
    family = LAGRANGE_VEC
    order = FIRST
  []
[]

[Functions]
  [primary_function]
    type = ParsedVectorFunction
    expression_x = '100*x'
    expression_y = '100*y'
    expression_z = '100*z'
  []

  [secondary_function]
    type = ParsedVectorFunction
    expression_x = '0'
    expression_y = '0'
    expression_z = '0'
  []

  [tertiary_function]
    type = ParsedVectorFunction
    expression_x = '100*x'
    expression_y = '100*y'
    expression_z = '100*z'
  []
[]

[ICs]
  [primary_ic]
    type = VectorFunctionIC
    variable = primary_lagrange_vector
    function = primary_function
  []

  [secondary_ic]
    type = VectorFunctionIC
    variable = secondary_lagrange_vector
    function = secondary_function  
  []

  [tertiary_ic]
    type = VectorFunctionIC
    variable = tertiary_lagrange_vector
    function = tertiary_function  
  []
[]

[Postprocessors]
  [primary_secondary_difference]
    type = ElementVectorL2Difference
    variable = primary_lagrange_vector
    other_variable = secondary_lagrange_vector
  []

  [primary_tertiary_difference]
    type = ElementVectorL2Difference
    variable = primary_lagrange_vector
    other_variable = tertiary_lagrange_vector
  []
[]

[Executioner]
  type = Transient
  dt = 1.0
  start_time = 0.0
  end_time = 1.0
[]

[Outputs]
  csv = true
[]
