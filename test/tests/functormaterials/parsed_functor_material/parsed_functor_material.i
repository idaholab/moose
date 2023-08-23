[Mesh]
  type = GeneratedMesh
  dim = 3
  nx = 2
  ny = 2
  nz = 2
  xmin = 0.0
  xmax = 4.0
  ymin = 0.0
  ymax = 6.0
  zmin = 0.0
  zmax = 10.0
[]

[Functions]
  [fn1]
    type = ConstantFunction
    value = 5
  []
  [fn2]
    type = ConstantFunction
    value = 3
  []
[]

[FunctorMaterials]
  [parsed_fmat]
    type = ParsedFunctorMaterial
    expression = 'A * B^2 + 2 + pi + e + t + x + y + z'
    functor_names = 'fn1 fn2'
    functor_symbols = 'A B'
    property_name = 'prop1'
  []
[]

[Postprocessors]
  # The value should be:
  # 5 * 3^2 + 2 + pi + e + 4 + 3 + 4.5 + 7.5 = 71.85987448204884
  [get_prop1]
    type = ElementExtremeFunctorValue
    functor = prop1
    value_type = max
    execute_on = 'INITIAL'
  []
[]

[Problem]
  solve = false
[]

[Executioner]
  type = Steady
  time = 4.0
[]

[Outputs]
  csv = true
  execute_on = 'INITIAL'
[]
