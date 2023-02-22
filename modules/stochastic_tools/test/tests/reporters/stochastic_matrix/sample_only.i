[StochasticTools]
[]

[Samplers/sample]
  type = InputMatrix
  matrix = '11 12 13 14;
            21 22 23 24;
            31 32 33 34;
            41 42 43 44;
            51 52 53 54'
[]

[Reporters/matrix]
  type = StochasticMatrix
  sampler = sample
  parallel_type = ROOT
[]

[Outputs]
  json = true
  execute_on = timestep_end
[]
