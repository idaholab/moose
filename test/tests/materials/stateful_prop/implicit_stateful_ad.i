# This test checks that material property for which the old state has been
# received first can still be delcared as AD

[Mesh]
  type = GeneratedMesh
  dim = 1
  xmin = 0
  xmax = 1
  nx = 10
[]

[Problem]
  kernel_coverage_check = false
  solve = false
[]

[Variables]
  [u]
  []
[]

[Materials]
  [stateful1]
    type = BadStatefulMaterial
  []
  [mat]
    type = ADGenericConstantMaterial
    prop_names = 'nonexistingpropertyname'
    prop_values = '42'
  []
[]

[Executioner]
  type = Steady
[]
