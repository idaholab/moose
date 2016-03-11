[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 1
  ny = 1
[]

[Materials]
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

[Debug]
  show_material_props = true
[]

[Outputs]
  exodus = true
[]
