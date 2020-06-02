[StochasticTools]
[]

[Distributions]
  [k_dist]
    type = Uniform
    lower_bound = 1
    upper_bound = 10
  []
  [q_dist]
    type = Uniform
    lower_bound = 9000
    upper_bound = 11000
  []
  [L_dist]
    type = Uniform
    lower_bound = 0.01
    upper_bound = 0.05
  []
  [Tinf_dist]
    type = Uniform
    lower_bound = 290
    upper_bound = 310
  []
[]

[Samplers]
  [sample]
    type = MonteCarlo
    num_rows = 100000
    distributions = 'k_dist q_dist L_dist Tinf_dist'
    execute_on = initial
  []
[]

[Surrogates]
  [gauss_process_avg]
    type = GaussianProcess
    filename = 'gauss_process_training_gauss_process_avg.rd'
  []
  [gauss_process_max]
    type = GaussianProcess
    filename = 'gauss_process_training_gauss_process_max.rd'
  []
[]

# # Computing statistics
[VectorPostprocessors]
  [samp_avg]
    type = SurrogateTester
    model = gauss_process_avg
    sampler = sample
  []
  [samp_max]
    type = SurrogateTester
    model = gauss_process_max
    sampler = sample
  []
#   [stats_avg]
#     type = PolynomialChaosStatistics
#     pc_name = 'gauss_process_avg'
#     compute = 'mean stddev'
#   []
#   [stats_max]
#     type = PolynomialChaosStatistics
#     pc_name = 'gauss_process_max'
#     compute = 'mean stddev'
#   []
#   [sense_avg]
#     type = PolynomialChaosLocalSensitivity
#     pc_name = 'gauss_process_avg'
#     local_points = '5 10000 0.03 300'
#   []
#   [sense_max]
#     type = PolynomialChaosLocalSensitivity
#     pc_name = 'gauss_process_max'
#     local_points = '5 10000 0.03 300'
#   []
#   [sobol_avg]
#     type = PolynomialChaosSobolStatistics
#     pc_name = 'gauss_process_avg'
#     sensitivity_order = 'first second'
#   []
#   [sobol_max]
#     type = PolynomialChaosSobolStatistics
#     pc_name = 'gauss_process_max'
#     sensitivity_order = 'first second'
#   []
[]

[Outputs]
  csv = true
[]
