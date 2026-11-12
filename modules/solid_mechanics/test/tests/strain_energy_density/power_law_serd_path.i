[Mesh]
  type = GeneratedMesh
  dim = 1
[]

[Problem]
  solve = false
[]

[Functions]
  # These deviatoric tensors prescribe effective stress t and effective strain rate t^2. For a
  # power-law exponent of two, the analytical strain energy rate density is 2*t^3/3.
  [stress_xx]
    type = ParsedFunction
    expression = '2*t/3'
  []
  [stress_yy]
    type = ParsedFunction
    expression = '-t/3'
  []
  [stress_zz]
    type = ParsedFunction
    expression = '-t/3'
  []
  [strain_rate_xx]
    type = ParsedFunction
    expression = 't^2'
  []
  [strain_rate_yy]
    type = ParsedFunction
    expression = '-t^2/2'
  []
  [strain_rate_zz]
    type = ParsedFunction
    expression = '-t^2/2'
  []
  [analytical_serd]
    type = ParsedFunction
    expression = '2*t^3/3'
  []
[]

[Materials]
  [stress]
    type = GenericFunctionRankTwoTensor
    tensor_name = stress
    tensor_functions = 'stress_xx stress_yy stress_zz'
  []
  [strain_rate]
    type = GenericFunctionRankTwoTensor
    tensor_name = strain_rate
    tensor_functions = 'strain_rate_xx strain_rate_yy strain_rate_zz'
  []
[]

[Executioner]
  type = Transient
  dt = 0.01
  num_steps = 100
[]

[Postprocessors]
  [analytical_serd]
    type = FunctionValuePostprocessor
    function = analytical_serd
  []
  [numerical_serd]
    type = ElementAverageMaterialProperty
    mat_prop = strain_energy_rate_density
  []
[]

[Outputs]
  csv = true
  execute_on = FINAL
[]
