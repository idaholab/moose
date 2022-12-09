[StochasticTools]
[]

[Distributions/dist]
  type = Uniform
  lower_bound = 0
  upper_bound = 1
[]

[Samplers/morris]
  type = MorrisSampler
  distributions = 'dist dist dist dist dist dist'
  trajectories = 1024
  levels = 4
[]

[VectorPostprocessors/gfun]
  type = GFunction
  sampler = morris
  q_vector = '0 0.5 3 9 99 99'
  parallel_type = DISTRIBUTED
  execute_on = initial
[]

[Reporters/stat]
  type = MorrisReporter
  sampler = morris
  vectorpostprocessors = 'gfun'
  ci_levels = '0.025 0.05 0.1 0.16 0.5 0.84 0.9 0.95 0.975'
  ci_replicates = 1000
  execute_on = initial
[]

[Outputs]
  [out]
    type = JSON
    execute_on = initial
  []
[]
