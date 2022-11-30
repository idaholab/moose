[ParameterStudy]
  input = sub_pseudo_transient.i
  parameters = 'BCs/left/value BCs/right/value'
  quantities_of_interest = 'average/value'

  sampling_type = monte-carlo
  num_rows = 3
  distributions = 'uniform uniform'
  uniform_lower_bound = '100 1'
  uniform_upper_bound = '200 2'

  sampler_output_type = 'csv'
  qoi_output_type = 'csv'
[]
