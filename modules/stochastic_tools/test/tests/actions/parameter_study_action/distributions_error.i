# This input is not a representative example, it is meant for checking if errors are hit
[ParameterStudy]
  input = sub.i
  parameters = 'BCs/left/value BCs/right/value'
  quantities_of_interest = 'average/value'

  sampling_type = monte-carlo
  num_samples = 15
[]
