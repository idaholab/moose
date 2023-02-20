[ParameterStudy]
  input = sub.i
  parameters = 'BCs/left/value BCs/right/value'
  quantities_of_interest = 'average/value'

  sampling_type = monte-carlo
  num_samples = 15
  distributions = 'uniform uniform'
  uniform_lower_bound = '100 1'
  uniform_upper_bound = '200 2'

  output_type = 'csv'
[]
