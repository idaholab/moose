[StochasticTools]
[]

[Postprocessors]
  [rosenbrock]
    type = Rosenbrock
    x = '0 0 0'
  []
  [eggholder]
    type = Eggholder
    x = '0 0'
  []
[]

[Controls]
  [stm]
    type = SamplerReceiver
  []
[]
