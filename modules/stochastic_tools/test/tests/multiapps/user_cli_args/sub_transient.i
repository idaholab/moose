[StochasticTools]
[]

[Mesh]
  type = GeneratedMesh
  dim = 1
[]

[Postprocessors]
  [size]
    type = AverageElementSize
    execute_on = 'initial'
  []
  [function_val]
    type = FunctionValuePostprocessor
    function = fun
    scale_factor = 1.0
  []
[]

[Functions/fun]
  type = ConstantFunction
  value = 1.0
[]

[Executioner]
  type = Transient
  num_steps = 3
[]

[Controls/receiver]
  type = SamplerReceiver
[]
