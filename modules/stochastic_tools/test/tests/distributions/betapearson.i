[StochasticTools]
[]

[Distributions]
  [betapearson]
    type = BetaPearson
    a = 5
    b = 5
    location = 5
    scale = 2
  []
[]

[Postprocessors]
  [cdf]
    type = TestDistributionPostprocessor
    distribution = betapearson
    value = 6.5
    method = cdf
    execute_on = initial
  []
  [pdf]
    type = TestDistributionPostprocessor
    distribution = betapearson
    value = 6.5
    method = pdf
    execute_on = initial
  []
  [quantile]
    type = TestDistributionPostprocessor
    distribution = betapearson
    value = 0.5
    method = quantile
    execute_on = initial
  []
[]

[Outputs]
  execute_on = 'INITIAL'
  csv = true
[]
