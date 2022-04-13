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

[AuxVariables]
  [u]
    order = CONSTANT
    family = MONOMIAL
  []
[]

[AuxKernels]
  [aux]
    type = OptionalTestAux
    variable = u
    prop = prop
    adprop = adprop
    expect = true
    adexpect = true
  []
[]

[Executioner]
  type = Transient
  num_steps = 4
[]

[Outputs]
  exodus = true
[]
