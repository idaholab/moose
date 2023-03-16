[StochasticTools]
[]

[Distributions]
  [D_dist]
    type = Normal
    mean = 5
    standard_deviation = 0.5
  []
  [S_dist]
    type = Normal
    mean = 8
    standard_deviation = 0.7
  []
[]

[Samplers]
  [sample]
    type = MonteCarlo
    num_rows = 100
    distributions = 'D_dist S_dist'
    execute_on = timestep_end
  []
  [quadrature]
    type = Quadrature
    distributions = 'D_dist S_dist'
    execute_on = INITIAL
    order = 5
  []
[]

[MultiApps]
  [quad_sub]
    type = SamplerFullSolveMultiApp
    input_files = sub.i
    sampler = quadrature
    mode = batch-restore
  []
[]

[Transfers]
  [quad]
    type = SamplerParameterTransfer
    to_multi_app = quad_sub
    sampler = quadrature
    parameters = 'Materials/diffusivity/prop_values Materials/xs/prop_values'
  []
  [data]
    type = SamplerReporterTransfer
    from_multi_app = quad_sub
    sampler = quadrature
    stochastic_reporter = storage
    from_reporter = avg/value
  []
[]

[Reporters]
  [storage]
    type = StochasticReporter
    outputs = none
  []
  [pc_samp]
    type = EvaluateSurrogate
    model = poly_chaos
    sampler = sample
    parallel_type = ROOT
    execute_on = final
  []
[]

[Surrogates]
  [poly_chaos]
    type = PolynomialChaos
    trainer = poly_chaos
  []
[]

[Trainers]
  [poly_chaos]
    type = PolynomialChaosTrainer
    execute_on = timestep_end
    order = 5
    distributions = 'D_dist S_dist'
    sampler = quadrature
    response = storage/data:avg:value
  []
[]

[Outputs]
  [out]
    type = CSV
    execute_on = FINAL
  []
[]
