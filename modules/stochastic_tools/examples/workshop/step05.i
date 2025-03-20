[ParameterStudy]
  input = diffusion.i
  parameters = 'Materials/constant/prop_values
                Kernels/source/value
                BCs/left/value
                BCs/right/value'
  quantities_of_interest = 'T_avg/value q_left/value'

  sampling_type = lhs
  num_samples = 5000
  distributions = 'uniform normal normal weibull'
  uniform_lower_bound = 0.5
  uniform_upper_bound = 2.5
  weibull_location = -110
  weibull_scale = 20
  weibull_shape = 1
  normal_mean = '100 300'
  normal_standard_deviation = '25 45'

  statistics = 'mean stddev'
  ci_levels = '0.05 0.95'
[]
