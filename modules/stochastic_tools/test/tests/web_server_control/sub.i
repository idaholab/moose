[StochasticTools]
[]

[Postprocessors]
  [rosenbrock]
    type = OptimizationTestFunction
    function = rosen
    x = '0 0 0'
  []
  [eggholder]
    type = OptimizationTestFunction
    function = eggholder
    x = '0 0'
  []
[]

[Controls]
  [stm]
    type = SamplerReceiver
  []
[]
