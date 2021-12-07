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
[]

[UserObjects]
  [uo]
    type = OptionalTestUserObject
    prop = prop
    adprop = adprop
    expect = true
    adexpect = true
    gold_function = t+1+x
    ad_gold_function = t+10+y
  []
[]

[Executioner]
  type = Transient
  num_steps = 2
[]
