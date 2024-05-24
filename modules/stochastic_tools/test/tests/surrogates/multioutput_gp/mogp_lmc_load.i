[StochasticTools]
[]

[Distributions]
  [k_dist]
    type = Normal
    mean = 15.0
    standard_deviation = 2.0
  []
  [bc_dist]
    type = Normal
    mean = 1000.0
    standard_deviation = 100.0
  []
[]

[Samplers]
  [test]
    type = LatinHypercube
    num_rows = 5
    distributions = 'k_dist bc_dist'
    seed = 101
  []
[]

[Reporters]
  [eval_test]
    type = EvaluateSurrogate
    model = mogp
    response_type = vector_real
    parallel_type = ROOT
    execute_on = timestep_end
    sampler = test
    evaluate_std = true
  []
[]

[Surrogates]
  [mogp]
    type = GaussianProcessSurrogate
    filename = "mogp_lmc_tuned_surr_mogp_trainer.rd"
  []
[]

[VectorPostprocessors]
  [test_params]
    type = SamplerData
    sampler = test
    execute_on = 'final'
  []
  [hyperparams]
    type = GaussianProcessData
    gp_name = mogp
    execute_on = final
  []
[]

[Outputs]
  [out]
    type = JSON
    execute_on = final
    vectorpostprocessors_as_reporters = true
    execute_system_information_on = NONE
  []
[]
