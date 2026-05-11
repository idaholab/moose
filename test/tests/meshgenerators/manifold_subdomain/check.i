[VectorPostprocessors]
  [elem_counter]
    type = ElementCounterWithID
    id_name = subdomain_id
    execute_on = 'initial'
  []
  [expected_elem]
    type = ConstantVectorPostprocessor
    value = '${expected}'
    execute_on = 'initial'
  []
[]

[Postprocessors]
  [compare]
    type = VectorPostprocessorComparison
    comparison_type = EQUALS
    vectorpostprocessor_a = elem_counter
    vector_name_a = nelem
    vectorpostprocessor_b = expected_elem
    vector_name_b = value
    execute_on = 'initial'
  []
[]

[UserObjects]
  [terminator]
    type = Terminator
    expression = 'compare < 0.5'
    error_level = ERROR
    message = 'Manifold did not produce expected number of elements in each subdomain: ${expected}'
  []
[]

[Problem]
  kernel_coverage_check = false
  solve = false
[]

[Executioner]
  type = Steady
[]
