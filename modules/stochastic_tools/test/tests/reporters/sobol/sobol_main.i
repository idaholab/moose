[StochasticTools]
[]

[Distributions/uniform]
  type = Uniform
  lower_bound = 0
  upper_bound = 1
[]

[Samplers]
  [sample]
    type = MonteCarlo
    distributions = 'uniform uniform uniform uniform uniform uniform'
    num_rows = 10
    seed = 0
    execute_on = PRE_MULTIAPP_SETUP
  []
  [resample]
    type = MonteCarlo
    distributions = 'uniform uniform uniform uniform uniform uniform'
    num_rows = 10
    seed = 1
    execute_on = PRE_MULTIAPP_SETUP
  []
  [sobol]
    type = Sobol
    sampler_a = sample
    sampler_b = resample
    execute_on = PRE_MULTIAPP_SETUP
  []
[]

[GlobalParams]
  sampler = sobol
[]

[MultiApps/sub]
  type = SamplerFullSolveMultiApp
  input_files = sub.i
  mode = batch-reset
[]

[Controls/param]
  type = MultiAppSamplerControl
  multi_app = sub
  param_names = 'x0 x1 x2 x3 x4 x5'
[]

[Transfers/data]
  type = SamplerReporterTransfer
  from_multi_app = sub
  from_reporter = 'const/gf const/gfa const/gf_vec'
  stochastic_reporter = storage
[]

[Reporters]
  [storage]
    type = StochasticReporter
    outputs = NONE
  []
  [sobol]
    type = SobolReporter
    reporters = 'storage/data:const:gf storage/data:const:gfa storage/data:const:gf_vec'
    ci_levels = '0.1 0.9'
    ci_replicates = 1000
    execute_on = FINAL
  []
[]

[Outputs]
  execute_on = FINAL
  [out]
    type = JSON
  []
[]
