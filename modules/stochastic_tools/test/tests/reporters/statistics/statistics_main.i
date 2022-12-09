[StochasticTools]
[]

[Samplers/sample]
  type = CartesianProduct
  linear_space_items = '1 1 6'
  execute_on = PRE_MULTIAPP_SETUP
[]

[GlobalParams]
  sampler = sample
[]

[MultiApps/sub]
  type = SamplerFullSolveMultiApp
  input_files = constant_sub.i
  mode = batch-reset
[]

[Controls/param]
  type = MultiAppSamplerControl
  multi_app = sub
  param_names = 'val'
[]

[Transfers/data]
  type = SamplerReporterTransfer
  from_multi_app = sub
  from_reporter = 'const/num const/int const/vec'
  stochastic_reporter = 'storage'
[]

[Reporters]
  [storage]
    type = StochasticReporter
    outputs = none
  []
  [stats]
    type = StatisticsReporter
    reporters = 'storage/data:const:num storage/data:const:int storage/data:const:vec'
    compute = 'min max sum mean stddev norm2 ratio stderr median'
  []
[]

[Outputs]
  execute_on = FINAL
  [out]
    type = JSON
  []
[]
