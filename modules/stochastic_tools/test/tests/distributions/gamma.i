[StochasticTools]
[]

[Distributions]
  [gamma1]
    type = Gamma
    shape = 7.7229537456741735
    scale = 1
  []
  [gamma2]
    type = Gamma
    shape = 0.606859544986121
    scale = 0.4477587035791184
  []
[]

[Postprocessors]
  [cdf1]
    type = TestDistributionPostprocessor
    distribution = gamma1
    value = 8.565580942828962
    method = cdf
    execute_on = initial
  []
  [pdf1]
    type = TestDistributionPostprocessor
    distribution = gamma1
    value = 5.25180066219824
    method = pdf
    execute_on = initial
  []
  [quantile1]
    type = TestDistributionPostprocessor
    distribution = gamma1
    value = 0.25545064031407105
    method = quantile
    execute_on = initial
  []
  [cdf2]
    type = TestDistributionPostprocessor
    distribution = gamma2
    value = 1
    method = cdf
    execute_on = initial
  []
  [pdf2]
    type = TestDistributionPostprocessor
    distribution = gamma2
    value = 1
    method = pdf
    execute_on = initial
  []
  [quantile2]
    type = TestDistributionPostprocessor
    distribution = gamma2
    value = 0.5
    method = quantile
    execute_on = initial
  []
[]

[Outputs]
  execute_on = 'INITIAL'
  csv = true
[]
