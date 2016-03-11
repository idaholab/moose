[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 2
  ny = 2
[]

[Materials]
  # fetch the properties first...
  [./client]
    type = DerivativeMaterialInterfaceTestClient
    block = 0
  [../]

  # ...then declare them!
  [./provider]
    type = DerivativeMaterialInterfaceTestProvider
    block = 0
    outputs = exodus
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
