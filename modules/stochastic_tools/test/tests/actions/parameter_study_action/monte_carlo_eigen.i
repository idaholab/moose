[ParameterStudy]
  input = sub_eigen.i
  parameters = 'Functions/diff_fun/value Functions/react_fun/value'
  quantities_of_interest = 'eigenvalue/value'

  sampling_type = monte-carlo
  num_rows = 5
  distributions = 'uniform uniform'
  uniform_lower_bound = '1 1'
  uniform_upper_bound = '2 2'

  sampler_output_type = 'csv'
  qoi_output_type = 'csv'
[]
