# NEML2 pure-elastic traction-separation law for the shifted cohesive zone method.
[Models]
  [decompose]
    type = VecComponents
    from = 'interface_displacement_jump'
    to = 'normal_separation tangential_separation_1 tangential_separation_2'
  []
  [traction]
    type = OrthotropicLinearTraction
    traction = 'interface_traction'
    normal_stiffness = 2e3
    tangential_stiffness = 1e3
  []
  [czm]
    type = ComposedModel
    models = 'decompose traction'
  []
[]
