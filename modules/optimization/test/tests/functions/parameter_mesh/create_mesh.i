[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 2
    ny = 2
  []
  parallel_type = REPLICATED
[]

[AuxVariables/params]
[]

[ICs/params_ic]
  type = FunctionIC
  function = params_fun
  variable = params
[]

[Functions]
  [params_fun]
    type = ParsedFunction
    expression = 'x*(x-1)*y*(y-1)'
  []
[]

[VectorPostprocessors]
  [param_vec]
    type = NodalValueSampler
    sort_by = id
    variable = params
  []
[]

[Outputs]
  csv = true
  exodus = true
  execute_on = timestep_end
[]

[Executioner]
  type = Steady
[]

[Problem]
  solve = false
[]
