# Boltzmann constant in J/K
k_B = 1.380649e-23
mass = 2.0
temperature = '${fparse mass / k_B }'

[StochasticTools]
[]

[Distributions]
  [distribution]
    type = Maxwellian
    mass = ${mass}
    temperature = ${temperature}
  []
[]

[Postprocessors]
  [cdf]
    type = TestDistributionPostprocessor
    distribution = distribution
    value = 0
    method = cdf
    execute_on = initial
  []
  [pdf]
    type = TestDistributionPostprocessor
    distribution = distribution
    value = 0
    method = pdf
    execute_on = initial
  []
  [quantile]
    type = TestDistributionPostprocessor
    distribution = distribution
    value = 0.5
    method = quantile
    execute_on = initial
  []
[]

[Outputs]
  execute_on = 'INITIAL'
  csv = true
[]
