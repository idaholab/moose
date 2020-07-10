[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 1
  ny = 1
[]

[Materials]
  [./provider]
    type = ADDerivativeMaterialInterfaceTestProvider
    block = 0
    outputs = exodus
    output_properties = 'dprop/db dprop/da d^2prop/dadb d^2prop/dadc d^3prop/dadbdc'
  [../]
  [./client]
    type = ADDerivativeMaterialInterfaceTestClient
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
