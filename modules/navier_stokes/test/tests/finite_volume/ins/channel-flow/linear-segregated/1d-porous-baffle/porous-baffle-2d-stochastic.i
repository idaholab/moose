# StochasticTools driver for the porous baffle 2D case.
# Column order in the matrix is:
#   inlet_velocity  mu  form_factor  forch_coeff
# Edit the rows below to specify the samples you want to run.

[StochasticTools]
[]

[Samplers]
  [samples]
    type = InputMatrix
    execute_on = 'PRE_MULTIAPP_SETUP'
    matrix = '2 2e-3 1 10; 2 2e-3 2 20; 2 2e-3 3 30;'
  []
[]

[MultiApps]
  [study]
    type = SamplerFullSolveMultiApp
    sampler = samples
    input_files = 'porous-baffle-2d-param-study.i'
    mode = normal
  []
[]

[Controls]
  [param_control]
    type = MultiAppSamplerControl
    multi_app = study
    sampler = samples
    param_names = 'inlet_velocity mu form_factor forch_coeff'
  []
[]

[VectorPostprocessors]
  [sample_matrix]
    type = SamplerData
    sampler = samples
  []
[]

[Outputs]
  csv = true
  execute_on = FINAL
[]
