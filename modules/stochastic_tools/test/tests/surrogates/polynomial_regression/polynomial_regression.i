[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 1
[]

[Executioner]
  type = Steady
[]

[Problem]
  solve = false
  kernel_coverage_check = false
[]

[Distributions]
  [uniform]
    type = Uniform
    lower_bound = 0
    upper_bound = 1
  []
[]

[Samplers]
  [mc]
    type = MonteCarlo
    num_rows = 100
    distributions = 'uniform'
  []
  [values]
    type = CartesianProduct
    linear_space_items = '0 0.01 100'
  []
  [test]
    type = CartesianProduct
    linear_space_items = '0 0.1 11'
  []
[]

[VectorPostprocessors]
  [values]
    type = SamplerData
    sampler = values
  []
  [values_data]
    type = SamplerData
    sampler = mc
  []
  [results]
    type = SurrogateTester
    model = surrogate
    sampler = test
    execute_on = final
  []
  [test_data]
    type = SamplerData
    sampler = test
  []
[]

[Surrogates]
  [surrogate]
    type = PolynomialRegressionSurrogate
    trainer = train
  []
[]

[Trainers]
  [train]
    type = PolynomialRegressionTrainer
    sampler = mc
    results_vpp = values
    results_vector = values_0
  []
[]

[Outputs]
  csv = true
  execute_on = FINAL
[]
