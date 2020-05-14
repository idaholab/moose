[StochasticTools]
[]

[Distributions]
  [this_is_the_wrong_type]
    type = Uniform
    lower_bound = 0
    upper_bound = 1
  []
[]

[Postprocessors]
  [cdf]
    type = TestDistributionDirectPostprocessor
    distribution = this_is_the_wrong_type
    value = 0
    method = cdf
    execute_on = initial
  []
[]

[Outputs]
  execute_on = 'INITIAL'
  csv = true
[]
