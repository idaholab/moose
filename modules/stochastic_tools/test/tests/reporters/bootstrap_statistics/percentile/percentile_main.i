[StochasticTools]
[]

[Distributions]
  [n0]
    type = Normal
    mean = 0
    standard_deviation = 1
  []
  [n1]
    type = Normal
    mean = 1
    standard_deviation = 1
  []
  [n2]
    type = Normal
    mean = 2
    standard_deviation = 0.5
  []
  [n3]
    type = Normal
    mean = 3
    standard_deviation = 0.33333333333
  []
  [n4]
    type = Normal
    mean = 4
    standard_deviation = 0.25
  []
[]

[Samplers/sample]
  type = MonteCarlo
  distributions = 'n0 n1 n2 n3 n4'
  num_rows = 100
  execute_on = PRE_MULTIAPP_SETUP
[]

[GlobalParams]
  sampler = sample
[]

[MultiApps/sub]
  type = SamplerFullSolveMultiApp
  input_files = sub.i
  mode = batch-reset
[]

[Controls/param]
  type = MultiAppSamplerControl
  multi_app = sub
  param_names = 'Reporters/const/real_vector_values[0,1,2,3,4]'
[]

[Transfers/data]
  type = SamplerReporterTransfer
  from_multi_app = sub
  from_reporter = 'const/num_vec'
  stochastic_reporter = storage
[]

[Reporters]
  [storage]
    type = StochasticReporter
    outputs = none
  []
  [stats]
    type = StatisticsReporter
    reporters = storage/data:const:num_vec
    compute = 'mean stddev'
    ci_method = 'percentile'
    ci_levels = '0.025 0.05 0.1 0.16 0.5 0.84 0.9 0.95 0.975'
    ci_replicates = 10000
    ci_seed = 1945
    execute_on = FINAL
  []
[]

[Outputs]
  execute_on = FINAL
  [out]
    type = JSON
  []
[]
