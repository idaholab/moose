[StochasticTools]
[]

[Distributions]
  [uniform]
    type = Uniform
    lower_bound = -7.0
    upper_bound = 7.0
  [] 
[]

[Samplers]
  [test]
    type = MonteCarlo
    distributions = 'uniform uniform uniform uniform uniform'
    num_rows = 1000
    seed = 100
  []
[]

[VectorPostprocessors]
  [values]
    type = nDRosenbrock
    sampler = test
    execute_on = FINAL
    limiting_value = 0.0
    classify = true
  []
[]

[Surrogates]
  [surr]
    type = LibtorchANNSurrogate
    filename = 'test_nn_out_train.rd'
    classify = true
  []
[]

[Reporters]
  [results]
    type = EvaluateSurrogate
    model = surr
    sampler = test
    execute_on = FINAL
    # parallel_type = ROOT
  []
[]

[Outputs]
  csv = true
  execute_on = FINAL
[]
