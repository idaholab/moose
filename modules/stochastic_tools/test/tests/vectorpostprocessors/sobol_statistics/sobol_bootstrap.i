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
    num_rows = 1024
    seed = 0
  []
  [resample]
    type = MonteCarlo
    distributions = 'uniform uniform uniform uniform uniform uniform'
    num_rows = 1024
    seed = 1
  []
  [sobol]
    type = Sobol
    sampler_a = sample
    sampler_b = resample
  []
[]

[VectorPostprocessors]
  [results]
    type = GFunction
    sampler = sobol
    q_vector = '0 0.5 3 9 99 99'
    execute_on = INITIAL
    outputs = none
    parallel_type = DISTRIBUTED
  []
  [sobol]
    type = SobolStatistics
    sampler = sobol
    results = results
    ci_levels = '0.025 0.05 0.1 0.16 0.5 0.84 0.9 0.95 0.975'
    ci_replicates = 1000
    execute_on = FINAL
  []
[]

[Outputs]
  execute_on = 'FINAL'
  csv = true
[]
