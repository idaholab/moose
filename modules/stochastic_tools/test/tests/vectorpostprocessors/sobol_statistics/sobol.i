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
    execute_on = FINAL
  []
[]

[Outputs]
  execute_on = 'FINAL'
  csv = true
[]
