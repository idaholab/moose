[StochasticTools]
[]

[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 1
[]

[Variables]
  [u]
  []
[]

[Kernels]
  [diff]
    type = Diffusion
    variable = u
  []
[]

[Distributions]
  [uniform]
    type = Uniform
    lower_bound = 0
    upper_bound = 1
  []
[]

[Samplers]
  [sample]
    type = MonteCarlo
    num_rows = 1
    distributions = uniform
  []
[]

[Executioner]
  type = Steady
[]

[MultiApps]
  [sub]
    type = SamplerFullSolveMultiApp
    sampler = sample
    input_files = 'citations_sub.i'
  []
[]
