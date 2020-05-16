[StochasticTools]
[]

[Distributions]
  [uniform]
    type = Uniform
    lower_bound = 5
    upper_bound = 10
  []
[]

[Samplers]
  [sample]
    type = MonteCarlo
    num_rows = 3
    distributions = 'uniform'
    execute_on = 'PRE_MULTIAPP_SETUP'
  []
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
    param_names = 'Mesh/xmax'
  []
[]
