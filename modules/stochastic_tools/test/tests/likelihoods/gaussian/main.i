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
    to_control = 'stochastic'
  []
  [reporter_transfer]
    type = SamplerReporterTransfer
    from_reporter = 'average/value'
    stochastic_reporter = 'constant'
    from_multi_app = sub
    sampler = sample
  []
  [reporter_transfer2]
    type = SamplerReporterTransfer
    from_reporter = 'average/value'
    stochastic_reporter = 'constant2'
    from_multi_app = sub
    sampler = sample
  []
[]

[Reporters]
  [constant]
    type = StochasticReporter
  []
  [constant2]
    type = StochasticReporter
  []
  [likelihoodtest]
    type = TestLikelihood
    likelihoods = 'gaussian truncgaussian'
    model_predictions = 'constant/reporter_transfer:average:value constant2/reporter_transfer2:average:value'
    weights = '0.5 0.5'
  []
[]

[Likelihoods]
  [gaussian]
    type = GaussianIID
    noise = 0.2
    file_name = 'exp1.csv'
    log_likelihood=true
  []
  [truncgaussian]
    type = TruncatedGaussianIID
    noise = 0.2
    file_name = 'exp1.csv'
    log_likelihood=true
    lower_bound = '-2.0 -2.0'
    upper_bound = '2.0 2.0'
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
