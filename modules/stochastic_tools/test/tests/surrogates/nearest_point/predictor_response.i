[StochasticTools]
[]

[Samplers]
  [sample]
    type = CartesianProduct
    linear_space_items = '0 1 10
                          0 1 10
                          0 1 10'
  []
  [test]
    type = CartesianProduct
    linear_space_items = '0.25 1 10
                          0.25 1 10
                          0.25 1 10'
  []
[]

[VectorPostprocessors]
  [sampler_data]
    type = SamplerData
    sampler = sample
    parallel_type = DISTRIBUTED
  []
  [values]
    type = GFunction
    sampler = sample
    q_vector = '0 0 0'
    execute_on = INITIAL
    outputs = none
  []
[]

[Reporters]
  [results]
    type = EvaluateSurrogate
    model = surrogate
    sampler = test
    parallel_type = ROOT
    execute_on = final
  []
[]

[Trainers]
  [train]
    type = NearestPointTrainer
    sampler = sample
    predictors = 'sampler_data/sample_0'
    predictor_cols = '1 2'
    response = values/g_values
  []
[]

[Surrogates]
  [surrogate]
    type = NearestPointSurrogate
    trainer = train
  []
[]

[Outputs]
  csv = true
  execute_on = final
[]
