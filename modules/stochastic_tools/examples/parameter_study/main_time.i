[StochasticTools]
[]

[Distributions]
  [gamma]
    type = Uniform
    lower_bound = 0.5
    upper_bound = 2.5
  []
  [q_0]
    type = Weibull
    location = -110
    scale = 20
    shape = 1
  []
  [T_0]
    type = Normal
    mean = 300
    standard_deviation = 45
  []
  [s]
    type = Normal
    mean = 100
    standard_deviation = 25
  []
[]

[Samplers]
  [hypercube]
    type = LatinHypercube
    num_rows = 5000
    distributions = 'gamma q_0 T_0 s'
  []
[]

[MultiApps]
  [runner]
    type = SamplerTransientMultiApp
    sampler = hypercube
    input_files = 'diffusion_time.i'
    mode = batch-restore
  []
[]

[Transfers]
  [parameters]
    type = SamplerParameterTransfer
    to_multi_app = runner
    sampler = hypercube
    parameters = 'Materials/constant/prop_values Kernels/source/value BCs/right/value BCs/left/value'
  []
  [results]
    type = SamplerReporterTransfer
    from_multi_app = runner
    sampler = hypercube
    stochastic_reporter = results
    from_reporter = 'T_avg/value q_left/value T_vec/T'
  []
  [x_transfer]
    type = MultiAppReporterTransfer
    from_multi_app = runner
    subapp_index = 0
    from_reporters = T_vec/x
    to_reporters = const/x
  []
[]

[Reporters]
  [results]
    type = StochasticReporter
    outputs = none
  []
  [stats]
    type = StatisticsReporter
    reporters = 'results/results:T_avg:value results/results:q_left:value results/results:T_vec:T'
    compute = 'mean stddev'
    ci_method = 'percentile'
    ci_levels = '0.05 0.95'
  []
  [const]
    type = ConstantReporter
    real_vector_names = 'x'
    real_vector_values = '0'
  []
[]

[Executioner]
  type = Transient
  solve_type = NEWTON
  num_steps = 4
  dt = 0.25
[]

[Outputs]
  execute_on = timestep_end
  [out]
    type = JSON
  []
[]
