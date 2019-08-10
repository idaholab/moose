[Mesh]
  type = GeneratedMesh
  dim = 1
[]

[Variables]
  [./dummy]
  [../]
[]

[Kernels]
  [./dummy]
    type = Diffusion
    variable = dummy
  [../]
[]

[Materials]
  [./before]
    type = GenericConstantMaterial
    prop_names = 'before'
    prop_values = 1
  [../]
  [./test]
    type = HasMaterialTest
  [../]
  [./after]
    type = GenericConstantMaterial
    prop_names = 'after'
    prop_values = 1
  [../]
[]

[Executioner]
  type = Steady
[]
