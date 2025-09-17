[StochasticTools]
[]

[Samplers]
  [sample]
    type = InputMatrix
    matrix = '0 1 2 3'
    execute_on = 'INITIAL TIMESTEP_END'
  []
[]

[Functions]
  [dynamic_num_rows]
    type = ParsedFunction
    expression = '2 * t'
  []
  [sample_func]
    type = ParsedFunction
    expression = '100 * t + x * 4 + y'
  []
[]

[Controls]
  [sample_control]
    type = InputMatrixControl
    num_rows_function = dynamic_num_rows
    num_cols_function = '4'
    sample_function = sample_func
    parameter = 'Samplers/sample/matrix'
    execute_on = 'TIMESTEP_BEGIN'
  []
[]

[Reporters]
  [data]
    type = StochasticMatrix
    sampler = sample
    execute_on = 'INITIAL TIMESTEP_END'
    parallel_type = ROOT
  []
[]

[Executioner]
  type = Transient
  num_steps = 3
  dt = 1
[]

[Outputs]
  json = true
[]
