[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 1
  []
[]

[Problem]
  solve = false
[]

[UserObjects]
  [dummy]
    type = TimestepSize
  []
[]

[Executioner]
  type = Steady
[]

[AuxVariables]
  [v]
  []
[]

[AuxKernels]
  [gap]
    type = MortarUserObjectAux
    variable = v
    user_object = dummy
    # contact_quantity =
  []
[]
