[StochasticTools]
[]

[Samplers]
  [grid]
    type = CartesianProduct
    execute_on = PRE_MULTIAPP_SETUP
    # Grid spacing:
    # k:    1.45  2.35  3.25  4.15  5.05  5.95  6.85  7.75  8.65  9.55
    # q:    9100  9300  9500  9700  9900  10100 10300 10500 10700 10900
    # L:    0.012 0.016 0.020 0.024 0.028 0.032 0.036 0.040 0.044 0.048
    # Tinf: 291   293   295   297   299   301   303   305   307   309
    linear_space_items = '1.45  0.9   10
                          9100  200   10
                          0.012 0.004 10
                          291   2     10'
  []
[]

[MultiApps]
  [sub]
    type = SamplerFullSolveMultiApp
    input_files = sub.i
    sampler = grid
  []
[]

[Controls]
  [cmdline]
    type = MultiAppSamplerControl
    multi_app = sub
    sampler = grid
    param_names = 'Materials/conductivity/prop_values Kernels/source/value Mesh/xmax BCs/right/value'
  []
[]

[Transfers]
  [data]
    type = SamplerReporterTransfer
    from_multi_app = sub
    sampler = grid
    stochastic_reporter = results
    from_reporter = 'avg/value max/value'
  []
[]

[Reporters]
  [results]
    type = StochasticMatrix
    sampler = grid
    outputs = none
  []
[]

[Trainers]
  [nearest_point_avg]
    type = NearestPointTrainer
    execute_on = timestep_end
    sampler = grid
    predictors = 'results/grid_0'
    predictor_cols = '1 2 3'
    response = results/data:avg:value
  []
  [nearest_point_max]
    type = NearestPointTrainer
    execute_on = timestep_end
    sampler = grid
    response = results/data:max:value
  []
[]

[Outputs]
  [out]
    type = SurrogateTrainerOutput
    trainers = 'nearest_point_avg nearest_point_max'
    execute_on = FINAL
  []
[]
