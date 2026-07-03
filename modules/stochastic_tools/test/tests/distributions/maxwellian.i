# Boltzmann constant in J/K
k_B = 1.380649e-23

[StochasticTools]
[]

[Distributions]
  [distribution]
    type = Maxwellian
    mass = 2.0
    # this combination of mass and temperature
    # result in a distribution equivalent to a
    # normal distribution with a mean of 0 and
    # a standard deviation of 1
    temperature = ${fparse 2.0 / k_B}
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
