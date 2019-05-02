[Mesh]
  type = GeneratedMesh
  dim = 1
[]

[Problem]
  type = FEProblem
  solve = false
[]

[Executioner]
  type = Steady
[]

[Distributions]
  [./uniform]
    type = UniformDistribution
    lower_bound = 5
    upper_bound = 10
  [../]
[]

[Samplers]
  [./sample]
    type = MonteCarloSampler
    n_samples = 3
    distributions = 'uniform uniform'
    execute_on = 'initial timestep_end'
  [../]
[]

[MultiApps]
  [sub]
    type = FullSolveMultiApp
    positions = '0 0 0
                 1 1 1
                 2 2 2'
    input_files = 'sub.i'
  []
[]

[Controls]
  [cmdline]
    type = MultiAppCommandLineControl
    multi_app = sub
    sampler = sample
    param_names = 'Mesh/xmax Mesh/ymax Mesh/zmax'
  []
[]
