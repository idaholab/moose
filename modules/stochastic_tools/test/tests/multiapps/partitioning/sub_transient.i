[StochasticTools]
[]

[Postprocessors]
  [pp1]
    type = FunctionValuePostprocessor
    function = 't'
    scale_factor = 1
  []
  [pp2]
    type = FunctionValuePostprocessor
    function = 't'
    scale_factor = 1
  []
[]

[Executioner]
  type = Transient
  num_steps = 3
[]

[Controls/receiver]
  type = SamplerReceiver
[]
