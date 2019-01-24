[Mesh]
  type = GeneratedMesh
  dim = 1
[]

[Problem]
  solve = false
[]

[Executioner]
  type = Transient
  dt = 1
  num_steps = 4
[]

[Functions]
  [func]
    type = ConstantFunction
    value = 5
  []
[]

[Postprocessors]
  [timestep_end]
    type = FunctionValuePostprocessor
    function = 't'
    execute_on = 'initial timestep_end'
  []
  [final]
    type = FunctionValuePostprocessor
    function = '2*t'
    execute_on = 'final'
  []
[]

[Outputs]
  csv = true
  [on_final]
    type = CSV
    execute_on = final
  []
[]
