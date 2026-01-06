[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 4
  xmin = 0
  xmax = 1
[]

[Functions]
  [fn]
    type = PiecewiseConstant
    axis = x
    x = '0 0.25 0.50 0.75'
    y = '5 2 3 4'
  []
  [fn2]
    type = ParsedFunction
    expression = x
  []
  [fn3]
    type = ParsedFunction
    expression = 1-x
  []
[]

[Materials]
  [mat]
    type = GenericFunctionMaterial
    prop_names = 'mat_prop'
    prop_values = 'fn'
  []
  [mat2]
    type = GenericFunctionMaterial
    prop_names = 'mat_prop2'
    prop_values = 'fn2'
  []
  [mat3]
    type = GenericFunctionMaterial
    prop_names = 'mat_prop3'
    prop_values = 'fn3'
  []
[]

[Reporters]
  [min]
    type = ElementExtremeMaterialPropertyReporter
    material_property = mat_prop
    value_type = min
    execute_on = 'INITIAL'
    additional_reported_properties = 'mat_prop2 mat_prop3'
  []
  [max]
    type = ElementExtremeMaterialPropertyReporter
    material_property = mat_prop
    value_type = max
    execute_on = 'INITIAL'
    additional_reported_properties = 'mat_prop2 mat_prop3'
  []
[]

[Problem]
  solve = false
[]

[Executioner]
  type = Steady
[]

[Outputs]
  json = true
  execute_on = 'INITIAL'
[]
