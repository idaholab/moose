[StochasticTools]
[]

[Distributions]
  [uniform]
    type = Uniform
    lower_bound = 1
    upper_bound = 10
  []
[]

[Samplers]
  [dynamic]
    type = MonteCarlo
    num_rows = 5
    distributions = 'uniform'
  []
[]

[MultiApps]
  [runner]
    type = SamplerFullSolveMultiApp
    sampler = dynamic
    input_files = 'sub.i'
  []
[]

[Transfers]
  [parameters]
    type = SamplerParameterTransfer
    to_multi_app = runner
    sampler = dynamic
    parameters = 'BCs/right/value'
  []
  [results]
    type = SamplerPostprocessorTransfer
    from_multi_app = runner
    sampler = dynamic
    to_vector_postprocessor = results
    from_postprocessor = 'center'
  []
[]

[Executioner]
  type = Transient
  num_steps = 2
[]

[VectorPostprocessors]
  [results]
    type = StochasticResults
  []
[]

[Outputs]
  [out]
    type = JSON
    vectorpostprocessors_as_reporters = true
  []
[]
