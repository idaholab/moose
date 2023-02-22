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
  [hypercube_a]
    type = LatinHypercube
    num_rows = 10000
    distributions = 'gamma q_0 T_0 s'
    seed = 2011
  []
  [hypercube_b]
    type = LatinHypercube
    num_rows = 10000
    distributions = 'gamma q_0 T_0 s'
    seed = 2013
  []
  [sobol]
    type = Sobol
    sampler_a = hypercube_a
    sampler_b = hypercube_b
  []
[]

[MultiApps]
  [runner]
    type = SamplerFullSolveMultiApp
    sampler = sobol
    input_files = 'diffusion.i'
    mode = batch-restore
  []
[]

[Transfers]
  [parameters]
    type = SamplerParameterTransfer
    to_multi_app = runner
    sampler = sobol
    parameters = 'Materials/constant/prop_values Kernels/source/value BCs/right/value BCs/left/value'
  []
  [results]
    type = SamplerReporterTransfer
    from_multi_app = runner
    sampler = sobol
    stochastic_reporter = results
    from_reporter = 'T_avg/value q_left/value'
  []
[]

[Reporters]
  [results]
    type = StochasticReporter
    outputs = none
  []
  [stats]
    type = StatisticsReporter
    reporters = 'results/results:T_avg:value results/results:q_left:value'
    compute = 'mean'
    ci_method = 'percentile'
    ci_levels = '0.05 0.95'
  []
  [sobol]
    type = SobolReporter
    sampler = sobol
    reporters = 'results/results:T_avg:value results/results:q_left:value'
    ci_levels = '0.05 0.95'
  []
[]

[Outputs]
  execute_on = 'FINAL'
  [out]
    type = JSON
  []
[]
