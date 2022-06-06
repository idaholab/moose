[StochasticTools]
[]

[Samplers/sample]
  type = CartesianProduct
  linear_space_items = '0 1 1'
[]

[Reporters]
  [constant]
    type = ConstantReporter
    real_vector_names = 'reporter_transfer:average:value'
    real_vector_values = '0'
  []
  [adaptive_MC]
    type = AdaptiveMonteCarloDecision
    output_value = constant/reporter_transfer:average:value
    inputs = 'inputs'
    sampler = sample
  []
[]
