[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 1
  ny = 1
[]

[Materials]
  [./const]
    type = GenericConstantMaterial
    block = 0
    prop_names  = 'constant_zero'
    prop_values = '0.0'
  [../]
  [./provider]
    type = DerivativeMaterialInterfaceTestProvider
    block = 0
    outputs = exodus
  [../]
  [./client]
    type = DerivativeMaterialInterfaceTestClient
    block = 0
  [../]
[]

[Executioner]
  type = Steady
[]

[Problem]
  solve = false
[]

[Outputs]
  output_initial = true
  exodus = true
  [./console]
    type = Console
    perf_log = false
  [../]
[]
