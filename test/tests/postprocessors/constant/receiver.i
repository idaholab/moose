[Mesh]
  [mesh]
    type = GeneratedMeshGenerator
    dim = 1
    nx = 1
  []
[]

[Problem]
  type = FEProblem
  solve = false
[]

[Executioner]
  type = Transient
  num_steps = 3
  dt = 1
[]

[Controls]
  [func_control]
    type = RealFunctionControl
    parameter = 'Postprocessors/recv/value'
    function = 'val'
    execute_on = 'timestep_begin'
  []
[]

[Functions]
  [val]
    type = ParsedFunction
    expression = '1 + 2*t'
  []
[]

[Postprocessors]
  [recv]
    type = ConstantPostprocessor
  []
[]

[Outputs]
  csv = true
[]
