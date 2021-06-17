[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 1
  []
[]

[Postprocessors]
  [scale]
    type = ScalePostprocessor
    value = function
    scaling_factor = 2
  []
  [function]
    type = FunctionValuePostprocessor
    function = 1
  []
[]

[VectorPostprocessors/constant_vpp]
  type = ConstantVectorPostprocessor
  vector_names = 'value1 value2'
  value = '1; 2'
[]

[Reporters/constant_reporter]
  type = ConstantReporter
  integer_names = integer
  integer_values = 1
  real_names = real
  real_values = 2
  string_names = string
  string_values = 'funny'
[]

[Debug]
  show_reporters = true
[]

[Problem]
  solve = false
[]

[Executioner]
  type = Steady
[]
