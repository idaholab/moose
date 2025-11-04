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
  []
  [resample]
    type = MonteCarlo
    distributions = 'uniform uniform uniform uniform uniform uniform'
    num_rows = 10
    seed = 1
  []
  [sobol]
    type = Sobol
    sampler_a = sample
    sampler_b = resample
    resample = false
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
[]

[Reporters]
  [sobol]
    type = SobolReporter
    sampler = sobol
    vectorpostprocessors = results
    ci_levels = '0.1 0.9'
    ci_replicates = 10
    execute_on = FINAL
  []
[]

[Outputs]
  execute_on = 'FINAL'
  [out]
    type = JSON
  []
[]
