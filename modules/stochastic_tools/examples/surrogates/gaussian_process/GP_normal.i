[StochasticTools]
[]

[Distributions]
  [k_dist]
    type = TruncatedNormal
    mean = 5
    standard_deviation = 2
    lower_bound = 0
  []
  [q_dist]
    type = TruncatedNormal
    mean = 10000
    standard_deviation = 500
    lower_bound = 0
  []
  [L_dist]
    type = TruncatedNormal
    mean = 0.03
    standard_deviation = 0.01
    lower_bound = 0
  []
  [Tinf_dist]
    type = TruncatedNormal
    mean = 300
    standard_deviation = 10
    lower_bound = 0
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
    filename = 'GP_training_normal_GP_avg.rd'
  []
  [GP_max]
    type = GaussianProcess
    filename = 'GP_training_normal_GP_max.rd'
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
  [GP_avg_hyperparams]
    type = GaussianProcessData
    gp_name = 'GP_avg'
  []
  [GP_max_hyperparams]
    type = GaussianProcessData
    gp_name = 'GP_max'
  []
[]

[Outputs]
  csv = true
[]
