[StochasticTools]
[]

[Distributions]
  [D_dist]
    type = Uniform
    lower_bound = 2.5
    upper_bound = 7.5
  []
  [S_dist]
    type = Uniform
    lower_bound = 2.5
    upper_bound = 7.5
  []
[]

[Samplers]
  [sample]
    type = MonteCarlo
    num_rows = 100
    distributions = 'D_dist S_dist'
    execute_on = initial
  []
[]

[MultiApps]
  [quad_sub]
    type = SamplerFullSolveMultiApp
    input_files = sub.i
    sampler = sample
    mode = batch-restore
  []
[]

[Transfers]
  [quad]
    type = SamplerParameterTransfer
    multi_app = quad_sub
    sampler = sample
    parameters = 'Materials/diffusivity/prop_values Materials/xs/prop_values'
    to_control = 'stochastic'
  []
  [data]
    type = SamplerPostprocessorTransfer
    multi_app = quad_sub
    sampler = sample
    to_vector_postprocessor = storage
    from_postprocessor = avg
  []
[]

[VectorPostprocessors]
  [storage]
    type = StochasticResults
    parallel_type = REPLICATED
  []
  [pc_coeff]
    type = PolynomialChaosData
    pc_name = poly_chaos
    execute_on = final
  []
  [pc_samp]
    type = EvaluateSurrogate
    model = poly_chaos
    sampler = sample
    output_samples = true
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
    sampler = sample
    results_vpp = storage
    results_vector = data:avg
  []
[]

[Outputs]
  [out]
    type = CSV
    execute_on = FINAL
  []
[]
