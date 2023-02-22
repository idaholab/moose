[ParameterStudy]
  input = sub.i
  parameters = 'BCs/left/value BCs/right/value'
  quantities_of_interest = 'average/value'

  sampling_type = cartesian-product
  linear_space_items = '100 25   5
                        1   0.25 5'

  output_type = 'csv'
[]
