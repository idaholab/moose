[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 1
  []
[]

[UserObjects/special_type_test]
  type = ReporterSpecialTypeTest
  pp_reporter = "pp/value"
  vpp_reporter = "vpp/value"
[]

[Postprocessors/pp]
  type = FunctionValuePostprocessor
  function = 1
[]

[VectorPostprocessors/vpp]
  type = ConstantVectorPostprocessor
  vector_names = 'value'
  value = '2'
[]

[Problem]
  solve = false
[]

[Executioner]
  type = Steady
[]

[Outputs]
  csv = true
  execute_on = timestep_end
[]
