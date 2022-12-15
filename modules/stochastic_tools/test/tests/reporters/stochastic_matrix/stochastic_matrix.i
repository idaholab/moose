[StochasticTools]
[]

[Samplers]
  [sample]
    type = InputMatrix
    matrix = '11 12 13 14;
              21 22 23 24;
              31 32 33 34;
              41 42 43 44;
              51 52 53 54'
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
    parameters = 'Functions/afun/value Functions/bfun/value Functions/cfun/value Functions/dfun/value'
  []
  [data]
    type = SamplerReporterTransfer
    from_multi_app = sub
    sampler = sample
    from_reporter = val/value
    stochastic_reporter = matrix
  []
[]

[Reporters]
  [matrix]
    type = StochasticMatrix
    sampler = sample
    sampler_column_names = 'a b c d'
    parallel_type = ROOT
  []
[]

[Outputs]
  execute_on = timestep_end
  csv = true
[]
