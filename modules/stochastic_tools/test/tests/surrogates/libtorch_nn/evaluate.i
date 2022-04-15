[StochasticTools]
[]

[Samplers]
  [test]
    type = CartesianProduct
    linear_space_items = '0 0.05 2
                          0 0.05 2
                          0 0.05 2'
  []
[]

[Surrogates]
  [surr]
    type = LibtorchANNSurrogate
    filename = 'train_out_train.rd'
  []
[]

[Reporters]
  [results]
    type = EvaluateSurrogate
    model = surr
    sampler = test
    execute_on = FINAL
    parallel_type = ROOT
  []
[]

[Outputs]
  csv = true
  execute_on = FINAL
[]
