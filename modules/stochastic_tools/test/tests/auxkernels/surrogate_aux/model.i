[StochasticTools]
[]

[Postprocessors]
  [x1]
    type = ConstantPostprocessor
    value = 0
  []
  [x2]
    type = ConstantPostprocessor
    value = 0
  []
  [x3]
    type = ConstantPostprocessor
    value = 0
  []
  [x4]
    type = ConstantPostprocessor
    value = 0
  []

  [val]
    type = ParsedPostprocessor
    function = '1 +     x1 +    x2 +    x3 +    x4 +
                     x1*x1 + x1*x2 + x1*x3 + x1*x4 +
                             x2*x2 + x2*x3 + x2*x4 +
                                     x3*x3 + x3*x4 +
                                             x4*x4'
    pp_names = 'x1 x2 x3 x4'
  []
[]

[Controls/receiver]
  type = SamplerReceiver
[]

[Outputs]
  console = false
[]
