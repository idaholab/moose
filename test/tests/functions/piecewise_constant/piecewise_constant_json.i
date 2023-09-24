[Mesh]
  type = GeneratedMesh
  dim = 1
[]

[Problem]
  solve = false
[]

[UserObjects]
  [json]
    type = JSONFileReader
    filename = 'function_values.json'
  []
[]

[Functions]
  [from_json]
    type = PiecewiseConstant
    json_uo = 'json'
    x_keys = "los_datos some_key some_other_key"
    y_keys = "los_datos second_key some_other_key"
  []
[]

[Postprocessors]
  [from_json]
    type = FunctionValuePostprocessor
    function = from_json
    execute_on = 'TIMESTEP_END INITIAL'
  []
[]

[Executioner]
  type = Transient
  dt = 1
  start_time = 0
  end_time = 10
[]

[Outputs]
  csv = true
[]
