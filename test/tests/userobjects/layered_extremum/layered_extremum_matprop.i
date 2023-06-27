[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 6
  ny = 13
  nz = 6
[]

[AuxVariables]
  [layered_extremum]
    order = CONSTANT
    family = MONOMIAL
  []
[]

[AuxKernels]
  [liaux]
    type = SpatialUserObjectAux
    variable = layered_extremum
    user_object = layered_uo
    execute_on = 'INITIAL LINEAR'
  []
[]

[UserObjects]
  [layered_uo]
    type = LayeredExtremumMaterialProperty
    direction = y
    num_layers = 10
    mat_prop = mat
    value_type = 'min'
    execute_on = 'INITIAL LINEAR'
  []
[]

[Materials]
  [mat]
    type = GenericFunctionMaterial
    prop_names = 'mat'
    prop_values = 'linear_one'
    output_properties = 'mat'
    outputs = 'exodus'
  []
[]

[Functions]
  [linear_one]
    type = ParsedFunction
    expression = 'x + 2*y + 1'
  []
[]

[VectorPostprocessors]
  [output]
    type = SpatialUserObjectVectorPostprocessor
    userobject = layered_uo
  []
[]

[Executioner]
  type = Steady
[]

[Problem]
  solve = false
[]

[Outputs]
  file_base = out
  exodus = true
  csv = true
[]
