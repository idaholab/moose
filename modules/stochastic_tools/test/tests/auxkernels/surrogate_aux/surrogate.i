[StochasticTools]
[]

[Distributions]
  [uniform]
    type = Uniform
    lower_bound = 0
    upper_bound = 1
  []
[]

[Samplers]
  [mc]
    type = MonteCarlo
    distributions = 'uniform uniform uniform uniform'
    num_rows = 50
  []
[]

[GlobalParams]
  sampler = mc
[]

[MultiApps]
  [model]
    type = SamplerFullSolveMultiApp
    input_files = model.i
    mode = batch-restore
    no_backup_and_restore = true
  []
[]

[Transfers]
  [param]
    type = SamplerParameterTransfer
    to_multi_app = model
    parameters ='Postprocessors/x1/value Postprocessors/x2/value Postprocessors/x3/value Postprocessors/x4/value'
  []
  [data]
    type = SamplerReporterTransfer
    from_multi_app = model
    stochastic_reporter = storage
    from_reporter = 'val/value'
  []
[]

[Reporters]
  [storage]
    type = StochasticReporter
  []
[]

[Trainers]
  [poly_regression]
    type = PolynomialRegressionTrainer
    regression_type = ols
    max_degree = 2
    response = storage/data:val:value
  []
[]

[Outputs]
  [trainer]
    type = SurrogateTrainerOutput
    trainers = poly_regression
  []
[]
