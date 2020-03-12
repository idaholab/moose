[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 1
  ny = 1
[]

[Variables]
  [u]
  []
[]

[Distributions]
  [uniform_left]
    type = Uniform
    lower_bound = 0
    upper_bound = 0.5
  []
  [uniform_right]
    type = Uniform
    lower_bound = 1
    upper_bound = 2
  []
[]

[Samplers]
  [sobol]
    type = Sobol
    num_rows = 3
    distributions = 'uniform_left uniform_right'
    execute_on = INITIAL # create random numbers on initial and use them for each timestep
  []
  [mc]
    type = MonteCarlo
    num_rows = 5
    distributions = 'uniform_left uniform_right'
    execute_on = INITIAL # create random numbers on initial and use them for each timestep
  []
[]

[MultiApps]
  [sobol]
    type = SamplerTransientMultiApp
    input_files = sub.i
    sampler = sobol
  []
  [mc]
    type = SamplerTransientMultiApp
    input_files = sub.i
    sampler = mc
  []
[]

[Transfers]
  [sobol]
    type = SamplerParameterTransfer
    multi_app = sobol
    sampler = sobol
    parameters = 'BCs/left/value BCs/right/value'
    to_control = 'stochastic'
  []
  [sobol_data]
    type = SamplerPostprocessorTransfer
    multi_app = sobol
    sampler = sobol
    to_vector_postprocessor = storage
    from_postprocessor = avg
  []

  [mc]
    type = SamplerParameterTransfer
    multi_app = mc
    sampler = mc
    parameters = 'BCs/left/value BCs/right/value'
    to_control = 'stochastic'
  []
  [mc_data]
    type = SamplerPostprocessorTransfer
    multi_app = mc
    sampler = mc
    to_vector_postprocessor = storage
    from_postprocessor = avg
  []
[]

[VectorPostprocessors]
  [storage]
    type = StochasticResults
    parallel_type = REPLICATED
    samplers = 'sobol mc'
  []
[]

[Executioner]
  type = Transient
  num_steps = 1
[]

[Problem]
  solve = false
  kernel_coverage_check = false
[]

[Outputs]
  [out]
    type = CSV
    execute_on = FINAL
  []
[]
