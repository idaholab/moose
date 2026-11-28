# NEML2 bilinear mixed-mode traction-separation law for the shifted cohesive zone method.
[Models]
  [decompose]
    type = VecComponents
    from = 'interface_displacement_jump'
    to = 'signed_normal_separation tangential_separation_1 tangential_separation_2'
  []
  [tangential_separation]
    type = ScalarPNorm
    from = 'tangential_separation_1 tangential_separation_2'
    to = 'tangential_separation'
    exponent = 2
  []
  [macaulay_n]
    type = MacaulaySplit
    from = 'signed_normal_separation'
    to_positive = 'normal_separation'
    to_negative = 'normal_penetration'
  []
  [decompose_old]
    type = VecComponents
    from = 'interface_displacement_jump~1'
    to = 'signed_normal_separation_old tangential_separation_1_old tangential_separation_2_old'
  []
  [tangential_separation_old]
    type = ScalarPNorm
    from = 'tangential_separation_1_old tangential_separation_2_old'
    to = 'tangential_separation_old'
    exponent = 2
  []
  [macaulay_n_old]
    type = MacaulaySplit
    from = 'signed_normal_separation_old'
    to_positive = 'normal_separation_old'
    to_negative = 'normal_penetration_old'
  []
  [mode_mixity_old]
    type = ModeMixity
    normal_separation = 'normal_separation_old'
    tangential_separation = 'tangential_separation_old'
    mode_mixity = 'mode_mixity_old'
  []
  [critical_separation]
    type = CamanhoDavilaCriticalSeparation
    normal_separation = 'normal_separation_old'
    mode_mixity = 'mode_mixity_old'
    penalty_stiffness = 2e3
    normal_strength = 500
    shear_strength = 300
  []
  [full_separation]
    type = BenzeggaghKenaneFullSeparation
    normal_separation = 'normal_separation_old'
    mode_mixity = 'mode_mixity_old'
    critical_separation = 'critical_separation'
    penalty_stiffness = 2e3
    mode_I_fracture_toughness = 1e3
    mode_II_fracture_toughness = 1e2
    shear_strength = 300
    eta = 2.2
  []
  [effective_separation]
    type = ScalarPNorm
    from = 'normal_separation tangential_separation'
    to = 'effective_separation'
    exponent = 2
  []
  [traction]
    type = BilinearTraction
    traction = 'interface_traction'
    critical_separation = 'critical_separation'
    full_separation = 'full_separation'
    normal_penetration = 'normal_penetration'
    penalty_stiffness = 2e3
    viscosity = 0
  []
  [czm]
    type = ComposedModel
    models = 'decompose tangential_separation macaulay_n decompose_old tangential_separation_old macaulay_n_old mode_mixity_old critical_separation full_separation effective_separation traction'
    additional_outputs = 'damage'
  []
[]
