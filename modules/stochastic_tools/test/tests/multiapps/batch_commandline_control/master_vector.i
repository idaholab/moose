[StochasticTools]
[]

[Distributions]
  [uniform]
    type = Uniform
    lower_bound = 5
    upper_bound = 10
  []
[]

[Samplers]
  [sample]
    type = MonteCarlo
    num_rows = 3
    distributions = 'uniform uniform uniform uniform'
    execute_on = 'PRE_MULTIAPP_SETUP'
  []
[]

[MultiApps]
  [sub]
    type = SamplerFullSolveMultiApp
    sampler = sample
    input_files = 'sub.i'
    mode = batch-reset
  []
[]

[Transfers]
  [data]
    type = SamplerPostprocessorTransfer
    from_multi_app = sub
    sampler = sample
    to_vector_postprocessor = storage
    from_postprocessor = size
  []
  [prop_A]
    type = SamplerPostprocessorTransfer
    from_multi_app = sub
    sampler = sample
    to_vector_postprocessor = prop_A
    from_postprocessor = prop_A
  []
  [prop_B]
    type = SamplerPostprocessorTransfer
    from_multi_app = sub
    sampler = sample
    to_vector_postprocessor = prop_B
    from_postprocessor = prop_B
  []
  [prop_C]
    type = SamplerPostprocessorTransfer
    from_multi_app = sub
    sampler = sample
    to_vector_postprocessor = prop_C
    from_postprocessor = prop_C
  []
  [prop_D]
    type = SamplerPostprocessorTransfer
    from_multi_app = sub
    sampler = sample
    to_vector_postprocessor = prop_D
    from_postprocessor = prop_D
  []
[]

[VectorPostprocessors]
  [storage]
    type = StochasticResults
  []
  [prop_A]
    type = StochasticResults
  []
  [prop_B]
    type = StochasticResults
  []
  [prop_C]
    type = StochasticResults
  []
  [prop_D]
    type = StochasticResults
  []
  [sample_data]
    type = SamplerData
    sampler = sample
  []
[]

[Controls]
  [cmdline]
    type = MultiAppSamplerControl
    multi_app = sub
    sampler = sample
    param_names = 'Mesh/xmax[0] Materials/const/prop_values[1,(1.5),2,2] Mesh/ymax[3]'
  []
[]

[Outputs]
  [out]
    type = CSV
    execute_on = FINAL
  []
[]
