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
  [mc_sample]
    type = MonteCarlo
    num_rows = 6
    distributions = 'q_dist'
    execute_on = initial
  []
  [cart_sample]
    type = CartesianProduct
    linear_space_items = '9000 2 1000'
                          #9000 200 11'
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
    type = GaussianProcessTester
    model = gauss_process_avg
    sampler = cart_sample
    output_samples = true
  []
  [samp_max]
    type = GaussianProcessTester
    model = gauss_process_max
    sampler = cart_sample
    output_samples = true
  []
  [train_avg]
    type = GaussianProcessTester
    model = gauss_process_avg
    sampler = mc_sample
    output_samples = true
  []
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
# []

[Outputs]
  csv = true
[]
