[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 1
  ny = 1
[]

[Distributions]
  [uniform]
    type = Uniform
    lower_bound = 2004
    upper_bound = 2010
  []
  [normal]
    type = Normal
    mean = 1980
    standard_deviation = 3
  []
[]

[Samplers]
  [sample]
    type = LatinHypercube
    num_rows = 1000
    distributions = 'uniform normal'
    upper_limits  = '1       0.999'
    lower_limits  = '0       0.001'
    num_bins      = '6       7'
    execute_on = 'initial timestep_end'
  []
[]

[VectorPostprocessors]
  [data]
    type = SamplerData
    sampler = sample
    execute_on = 'initial timestep_end'
  []
[]

[Executioner]
  type = Steady
[]

[Problem]
  solve = false
  kernel_coverage_check = false
[]

[Outputs]
  execute_on = 'INITIAL TIMESTEP_END'
  csv = true
[]
