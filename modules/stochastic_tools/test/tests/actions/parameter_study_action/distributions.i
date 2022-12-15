[ParameterStudy]
  input = sub.i
  parameters = 'p0 p1 p2 p3 p4 p5 p6'
  quantities_of_interest = 'const/p0 const/p1 const/p2 const/p3 const/p4 const/p5 const/p6'

  sampling_type = monte-carlo
  num_samples = 15
  distributions = 'uniform normal normal uniform weibull lognormal tnormal'
  uniform_lower_bound = '100 1'
  uniform_upper_bound = '200 2'
  normal_mean = '5 6'
  normal_standard_deviation = '0.001 0.1'
  weibull_location = -120
  weibull_scale = 20
  weibull_shape = 1
  lognormal_location = -0.371
  lognormal_scale = 0.52
  tnormal_mean = 100
  tnormal_standard_deviation = 25
  tnormal_lower_bound = 50
  tnormal_upper_bound = 150

  output_type = 'csv'
[]
