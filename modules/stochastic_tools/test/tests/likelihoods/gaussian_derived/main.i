[StochasticTools]
[]

[Distributions]
  [mu1]
    type = Normal
    mean = 0.0
    standard_deviation = 0.5
  []
[]

[Samplers]
  [sample]
    type = MonteCarlo
    distributions = 'mu1 mu1'
    num_rows = 3
    seed = 2547
  []
[]

[MultiApps]
  [sub]
    type = SamplerFullSolveMultiApp
    input_files = sub.i
    sampler = sample
  []
[]

[Transfers]
  [param]
    type = SamplerParameterTransfer
    to_multi_app = sub
    sampler = sample
    parameters = 'BCs/left/value BCs/right/value'
    # to_control = 'stochastic'
  []
  [reporter_transfer]
    type = SamplerReporterTransfer
    from_reporter = 'average/value'
    stochastic_reporter = 'constant'
    from_multi_app = sub
    sampler = sample
  []
[]

[Reporters]
  [constant]
    type = StochasticReporter
  []
  [noise_specified]
    type = ConstantReporter
    real_names = 'noise_specified'
    real_values = '0.2'
  []
  [likelihoodtest]
    type = TestLikelihood
    likelihoods = 'gaussian'
    model_pred = constant/reporter_transfer:average:value
    sampler = sample
  []
[]

[Likelihood]
  [gaussian]
    type = Gaussian
    noise = 'noise_specified/noise_specified'
    file_name = 'exp1.csv'
    log_likelihood=true
  []
[]

[Executioner]
  type = Steady
[]

[Outputs]
  file_base ='loglikelihood_scalar'
  [out]
    type = JSON
    execute_system_information_on = NONE
  []
[]
