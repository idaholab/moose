[StochasticTools]
[]

[Distributions]
  [fdist]
    type = FDistribution
    df1 = 6
    df2 = 193
  []
[]

[Postprocessors]
  [cdf]
    type = TestDistributionPostprocessor
    distribution = fdist
    value = 1.334609
    method = cdf
    execute_on = initial
  []
  [pdf]
    type = TestDistributionPostprocessor
    distribution = fdist
    value = 1.334609
    method = pdf
    execute_on = initial
  []
  [pdf2]
    type = TestDistributionPostprocessor
    distribution = fdist
    value = 50.5780462078443
    method = pdf
    execute_on = initial
  []
  [quantile]
    type = TestDistributionPostprocessor
    distribution = fdist
    value = 0.7564803
    method = quantile
    execute_on = initial
  []
[]

[Outputs]
  execute_on = 'INITIAL'
  csv = true
[]
