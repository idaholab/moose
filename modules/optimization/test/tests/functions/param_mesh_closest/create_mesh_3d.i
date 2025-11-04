[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 3
    nx = 5
    ny = 5
    nz = 5
    zmin = -2
  []
  [rot]
    type = TransformGenerator
    input = gmg
    transform = ROTATE
    vector_value = '10 10 10'
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
    value = '0.5 + x + y + x*y'
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
