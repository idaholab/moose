[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 1
[]

[Samplers]
  [test]
    type = CartesianProduct
    linear_space_items = '0.25 1 10
                          0.25 1 10
                          0.25 1 10'
  []
[]

[VectorPostprocessors]
  [results]
    type = SurrogateTester
    model = surrogate
    sampler = test
    execute_on = final
  []
[]

[Surrogates]
  [surrogate]
    type = NearestPointSurrogate
    filename = 'train_out_train.rd'
  []
[]

[Executioner]
  type = Steady
[]

[Problem]
  solve = false
  kernel_coverage_check = false
[]

[Outputs]
  csv = true
  execute_on = FINAL
[]
