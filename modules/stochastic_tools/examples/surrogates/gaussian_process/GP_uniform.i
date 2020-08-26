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
  [GP_avg]
    type = GaussianProcess
    filename = 'GP_training_uniform_GP_avg.rd'
  []
  [GP_max]
    type = GaussianProcess
    filename = 'GP_training_uniform_GP_max.rd'
  []
[]

# Computing statistics
[VectorPostprocessors]
  [samp_avg]
    type = EvaluateSurrogate
    model = GP_avg
    sampler = sample
    output_samples = true
  []
  [samp_max]
    type = EvaluateSurrogate
    model = GP_max
    sampler = sample
    output_samples = true
  []
[]

[Outputs]
  csv = true
[]
