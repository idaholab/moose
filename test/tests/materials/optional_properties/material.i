[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 10
    ny = 10
  []
[]

[Problem]
  solve = false
[]

[Materials]
  [before]
    type = OptionalTestMaterial
    prop = prop
    adprop = adprop
    expect = true
    adexpect = true
    outputs = exodus
  []

  [prop]
    type = GenericFunctionMaterial
    prop_names = prop
    prop_values = t+1+x
  []
  [adprop]
    type = ADGenericFunctionMaterial
    prop_names = adprop
    prop_values = t+10+y
  []

  [after]
    type = OptionalTestMaterial
    prop = prop
    adprop = adprop
    expect = true
    adexpect = true
    outputs = exodus
  []
[]

[Executioner]
  type = Transient
  num_steps = 4
[]

[Outputs]
  exodus = true
[]
