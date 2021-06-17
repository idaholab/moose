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
    mean = -10
    standard_deviation = 1.5
  []
  [s]
    type = Normal
    mean = 1
    standard_deviation = 0.25
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
    multi_app = runner
    sampler = sobol
    parameters = 'Materials/constant/prop_values Kernels/source/value BCs/right/value BCs/left/value'
    to_control = 'stochastic'
  []
  [results]
    type = SamplerPostprocessorTransfer
    multi_app = runner
    sampler = sobol
    to_vector_postprocessor = results
    from_postprocessor = 'T_avg q_left'
  []
[]

[VectorPostprocessors]
  [results]
    type = StochasticResults
  []
  [samples]
    type = SamplerData
    sampler = sobol
  []
  [stats]
    type = Statistics
    vectorpostprocessors = results
    compute = 'mean'
    ci_method = 'percentile'
    ci_levels = '0.05'
  []
  [sobol]
    type = SobolStatistics
    sampler = sobol
    results = results
  []
[]

[Outputs]
  csv = true
  execute_on = 'FINAL'
[]
