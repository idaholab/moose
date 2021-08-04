[StochasticTools]
[]

[Samplers]
  [sample]
    type = CartesianProduct
    linear_space_items = '0 1 10'
  []
  [test]
    type = CartesianProduct
    linear_space_items = '0.25 0.5 20'
  []
[]

[VectorPostprocessors]
  [values]
    type = GFunction
    sampler = sample
    q_vector = '0'
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

[Surrogates]
  [surrogate]
    type = NearestPointSurrogate
    trainer = train
  []
[]

[Trainers]
  [train]
    type = NearestPointTrainer
    sampler = sample
    response = values/g_values
  []
[]

[Outputs]
  csv = true
  execute_on = FINAL
[]
