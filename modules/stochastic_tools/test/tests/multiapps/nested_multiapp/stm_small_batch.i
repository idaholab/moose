[StochasticTools]
[]

[Samplers/sample]
  type = InputMatrix
  matrix = '0  4  8  12 16;
            1  5  9  13 17;
            2  6  10 14 18;
            3  7  11 15 19'
[]

[GlobalParams]
  sampler = sample
[]

[MultiApps]
  [main]
    type = SamplerFullSolveMultiApp
    input_files = main.i
  []
[]

[Transfers]
  [param]
    type = SamplerParameterTransfer
    to_multi_app = main
    parameters = 'BCs/left/value sub:BCs/left/value sub:subsub:BCs/left/value sub:subsub0:BCs/right/value sub:subsub1:BCs/right/value'
  []
  [data]
    type = SamplerReporterTransfer
    from_multi_app = main
    stochastic_reporter = storage
    from_reporter = 'val/value receive/sub_val receive/subsub0_left_val receive/subsub1_left_val receive/subsub0_right_val receive/subsub1_right_val'
  []
[]

[Reporters/storage]
  type = StochasticReporter
  parallel_type = ROOT
[]
