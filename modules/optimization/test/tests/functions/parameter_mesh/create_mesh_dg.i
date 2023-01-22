[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 4
    ny = 4
  []
  parallel_type = REPLICATED
[]

[AuxVariables/params]
  family = MONOMIAL
  order = CONSTANT
[]

[ICs/params_ic]
  type = FunctionIC
  function = params_fun
  variable = params
[]

[Functions]
  [params_fun]
    type = ParsedFunction
    value = 'x*(x-1)*y*(y-1)'
  []
[]

[VectorPostprocessors]
  [param_vec]
    type = ElementValueSampler
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
