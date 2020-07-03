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
[]

[Samplers]
  [test_sample]
    type = MonteCarlo
    num_rows = 100
    distributions = 'k_dist q_dist'
    execute_on = PRE_MULTIAPP_SETUP
  []
[]

[VectorPostprocessors]
  [samp_avg]
    type = GaussianProcessTester
    model = GP_avg
    sampler = test_sample
    output_samples = true
    execute_on = final
  []

[]

[Surrogates]
  [GP_avg]
    type = GaussianProcess
    filename = 'gauss_process_training_GP_avg_trainer.rd'
    covariance_function='covar'
  []
[]

[Covariance]
  [covar]
    type=SquaredExponentialCovariance
    signal_variance = 1                       #Use a signal variance of 1 in the kernel
    noise_variance = 1e-6                     #A small amount of noise can help with numerical stability
    length_factor = '0.38971 0.38971'         #Select a length factor for each parameter (k and q)
  []
[]

[Outputs]
  [out]
    type = CSV
    execute_on = FINAL
  []
[]
