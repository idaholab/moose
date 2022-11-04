[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 5
  xmin = 0
  xmax = 5
[]

[Functions]
  [test_fn]
    type = ParsedFunction
    expression = 'x'
  []
[]

[Materials]
  [test_mat]
    type = ADGenericFunctionMaterial
    prop_names = 'test_prop'
    prop_values = 'test_fn'
  []
[]

[Problem]
  solve = false
[]

[Executioner]
  type = Steady
[]

[VectorPostprocessors]
  [test_vpp]
    type = ADSampler1DReal
    block = 0
    property = test_prop
    sort_by = x
    execute_on = 'INITIAL'
  []
[]

[Outputs]
  csv = true
  execute_on = 'INITIAL'
[]
