[Mesh]
  type = GeneratedMesh

  dim = 2

  xmin = 0
  xmax = 1

  ymin = 0
  ymax = 1

  nx = 4
  ny = 4
[]

[AuxVariables]
  [parsed]
    order = FIRST
    family = LAGRANGE_VEC
  []
  [parsed_elem]
    order = FIRST
    family = MONOMIAL_VEC
  []

  [coupled_regular]
    initial_condition = 1
  []
  [coupled_vector]
    family = LAGRANGE_VEC
    initial_condition = '3 2 1'
  []
  [coupled_vector_2]
    family = LAGRANGE_VEC
    initial_condition = '10 12 0'
  []
  [coupled_regular_elem]
    family = MONOMIAL
    initial_condition = 1
  []
  [coupled_vector_elem]
    family = MONOMIAL_VEC
    initial_condition = '3 2 1'
  []
  [coupled_vector_elem_2]
    family = MONOMIAL_VEC
    initial_condition = '10 12 0'
  []
[]

[AuxKernels]
  [parsed]
    type = ParsedVectorAux
    variable = parsed
    expression_x = 'coupled_regular + 2 * coupled_vector + y'
    expression_y = '2 + coupled_regular + 2 * coupled_vector_2 + x'
    expression_z = 'x + z + coupled_regular'
    coupled_variables = coupled_regular
    coupled_vector_variables = 'coupled_vector coupled_vector_2'
    use_xyzt = true
  []
  [parsed_elem]
    type = ParsedVectorAux
    variable = parsed_elem
    expression_x = 'coupled_regular_elem + 2 * coupled_vector_elem + y'
    expression_y = '2 + coupled_regular_elem + 2 * coupled_vector_elem_2 + x'
    coupled_variables = coupled_regular_elem
    coupled_vector_variables = 'coupled_vector_elem coupled_vector_elem_2'
    use_xyzt = true
  []
[]

[Executioner]
  type = Steady
[]

[Problem]
  solve = false
[]

[Outputs]
  file_base = out
  exodus = true
  hide = 'coupled_regular coupled_regular_elem coupled_vector coupled_vector_2 coupled_vector_elem coupled_vector_elem_2'
[]
