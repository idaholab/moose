[StochasticTools]
[]

[Functions]
  [afun]
    type = ConstantFunction
    value = 11
  []
  [bfun]
    type = ConstantFunction
    value = 22
  []
  [cfun]
    type = ConstantFunction
    value = 33
  []
  [dfun]
    type = ConstantFunction
    value = 44
  []
  [fun]
    type = ParsedFunction
    value = 'a*1000000 + b*10000 + c*100 + d'
    vars = 'a b c d'
    vals = 'afun bfun cfun dfun'
  []
[]

[Postprocessors/val]
  type = FunctionValuePostprocessor
  function = fun
[]

[Controls/receiver]
  type = SamplerReceiver
[]

[Outputs]
  console = false
[]
