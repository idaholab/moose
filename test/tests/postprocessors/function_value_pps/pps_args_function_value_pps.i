[Mesh]
  type = GeneratedMesh
  dim = 1
[]

[Problem]
  solve = false
[]

[Functions]
  [fn]
    type = ParsedFunction
    value = 't + 0.12 * x + 0.3 * y + 12 * z'
  []
[]

[Postprocessors]
  [time_pp]
    type = Receiver
    default = 12
  []

  [z_pp]
    type = FunctionValuePostprocessor
    function = 't'
  []

  [val]
    type = FunctionValuePostprocessor
    time = 'time_pp'
    point = '-1 0 z_pp'
    function = fn
    execute_on = 'initial timestep_end'
  []
[]

[Executioner]
  type = Transient
  num_steps = 5
[]

[Outputs]
  csv = true
[]
