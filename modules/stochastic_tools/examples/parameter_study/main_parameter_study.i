[ParameterStudy]
  input = diffusion.i
  parameters = 'Materials/constant/prop_values Kernels/source/value BCs/right/value BCs/left/value'
  quantities_of_interest = 'T_avg/value q_left/value'

  sampling_type = lhs
  num_samples = 5000
  distributions = 'uniform weibull normal normal'
  uniform_lower_bound = 0.5
  uniform_upper_bound = 2.5
  weibull_location = -110
  weibull_scale = 20
  weibull_shape = 1
  normal_mean = '300 100'
  normal_standard_deviation = '45 25'
[]
