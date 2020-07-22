[Mesh]
  type = FileMesh
  file = rectangle.e
[]

[Variables]
  [./c]
  [../]
[]

[Materials]
  [./mat1]
    type = ADDefaultMatPropConsumerMaterial
    block = 1
  [../]
  [./mat2]
    type = ADDefaultMatPropConsumerMaterial
    block = 2
  [../]
  [./mat1b]
    type = ADDefaultMatPropConsumerMaterial
    mat_prop = prop2
    block = 1
  [../]
  [./mat2b]
    type = ADDefaultMatPropConsumerMaterial
    mat_prop = prop2
    block = 2
  [../]

  [./generic]
    type = ADGenericConstantMaterial
    block = '1 2'
    prop_names = prop3
    prop_values = 9
  [../]

  [./mat1c]
    type = ADDefaultMatPropConsumerMaterial
    mat_prop = prop3
    block = 1
  [../]
  [./mat2c]
    type = ADDefaultMatPropConsumerMaterial
    mat_prop = prop3
    block = 2
  [../]
[]

[Kernels]
  [./kern1]
    type = ADDefaultMatPropConsumerKernel
    variable = c
    block = 1
  [../]
  [./kern2]
    type = ADDefaultMatPropConsumerKernel
    variable = c
    block = 2
  [../]
  [./kern1b]
    type = ADDefaultMatPropConsumerKernel
    variable = c
    mat_prop = prop3
    block = 1
  [../]
  [./kern2b]
    type = ADDefaultMatPropConsumerKernel
    variable = c
    mat_prop = prop3
    block = 2
  [../]
[]

[Executioner]
  type = Steady
[]

[Debug]
  show_material_props = true
[]

[Problem]
  solve = false
[]

[Outputs]
  exodus = true
[]
