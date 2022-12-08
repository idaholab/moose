[StochasticTools]
[]

[Distributions/uniform]
  type = Uniform
  lower_bound = 0
  upper_bound = 1
[]

[Samplers/morris]
  type = MorrisSampler
  distributions = 'uniform uniform uniform uniform uniform uniform'
  trajectories = 10
  levels = 4
  execute_on = PRE_MULTIAPP_SETUP
[]

[GlobalParams]
  sampler = morris
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
  [morris]
    type = MorrisReporter
    reporters = 'storage/data:const:gf storage/data:const:gfa storage/data:const:gf_vec'
    ci_levels = '0.1 0.9'
    ci_replicates = 1000
    execute_on = FINAL
  []
[]

[Outputs]
  [out]
    type = JSON
    execute_on = FINAL
  []
[]
