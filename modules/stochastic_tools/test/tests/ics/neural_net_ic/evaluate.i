# [StochasticTools]
# []
#
# [Samplers]
#   [test]
#     type = CartesianProduct
#     linear_space_items = '0.25 1 10
#                           0.25 1 10
#                           0.25 1 10'
#   []
# []
#
# [VectorPostprocessors]
#   [results]
#     type = SurrogateTester
#     model = surrogate
#     sampler = test
#     execute_on = final
#   []
# []

[Surrogates]
  [surrogate]
    type = NeuralNetworkIC
    filename = 'train_out_train.rd'
  []
[]

[Outputs]
  csv = true
  execute_on = FINAL
[]
