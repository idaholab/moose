[StochasticTools]
[]

[Distributions]
  [D]
    type = Uniform
    lower_bound = 0.5
    upper_bound = 2.5
  []
  [q]
    type = Normal
    mean = 100
    standard_deviation = 25
  []
  [T_0]
    type = Normal
    mean = 300
    standard_deviation = 45
  []
  [q_0]
    type = Weibull
    location = -110
    scale = 20
    shape = 1
  []
[]

[Samplers]
  [hypercube]
    type = LatinHypercube
    num_rows = 1000
    distributions = 'D q T_0 q_0'
  []
  [resample]
    type = LatinHypercube
    num_rows = 1000
    seed = 2025
    distributions = 'D q T_0 q_0'
  []
  [sobol]
    type = Sobol
    sampler_a = hypercube
    sampler_b = resample
    resample = false
  []
[]

[MultiApps]
  [runner]
    type = SamplerFullSolveMultiApp
    sampler = sobol
    input_files = 'diffusion.i'
    cli_args = 'Outputs/console=false'
    mode = batch-restore
  []
[]

[Transfers]
  [parameters]
    type = SamplerParameterTransfer
    to_multi_app = runner
    sampler = sobol
    parameters = 'Materials/constant/prop_values
                  Kernels/source/value
                  BCs/left/value
                  BCs/right/value'
  []
  [results]
    type = SamplerReporterTransfer
    from_multi_app = runner
    sampler = sobol
    stochastic_reporter = sampling_matrix
    from_reporter = 'T_avg/value q_left/value'
  []
[]

[Reporters]
  [sampling_matrix]
    type = StochasticMatrix
    sampler = sobol
    sampler_column_names = 'D q T_0 q_0'
    parallel_type = ROOT
  []
  [sobol]
    type = SobolReporter
    sampler = sobol
    reporters = 'sampling_matrix/results:T_avg:value sampling_matrix/results:q_left:value'
    ci_levels = '0.05 0.95'
  []
[]

[Outputs]
  json = true
[]
