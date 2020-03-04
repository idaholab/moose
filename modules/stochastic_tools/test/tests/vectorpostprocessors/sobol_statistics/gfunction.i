[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 1
[]

[Functions/g]
  type = GFunction
  x_vector = '0.1 0.2 0.3 0.4 0.5 0.6'
[]

[Postprocessors/y]
  type = FunctionValuePostprocessor
  function = g
  point = '0.5 0 0'
  execute_on = 'initial timestep_end'
[]

[Executioner]
  type = Steady
[]

[Problem]
  solve = false
  kernel_coverage_check = false
[]

[Outputs]
  console = false
[]

[Controls/stochastic]
  type = SamplerReceiver
[]
