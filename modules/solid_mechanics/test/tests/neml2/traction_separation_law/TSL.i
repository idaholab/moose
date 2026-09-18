# nil.
# Bilinear mixed-mode TSL (Camanho-Davila initiation + Benzeggagh-Kenane
# propagation) built from the NEML2 v3 traction-separation primitives and
# wired so that the MOOSE cohesive-zone interface kernel can consume it:
#   input  = interface_displacement_jump  (Vec, from CZMComputeDisplacementJump*)
#   output = interface_traction           (Vec, read by CZMComputeGlobalTraction*)
#           + damage                      (additional output, internal state)
# Parameters match modules/solid_mechanics/test/tests/cohesive_zone_model/
# bilinear_mixed.i. The old viscous regularization parameter of the retired
# BiLinearMixedModeTractionSeparation model has no v3 equivalent (this law is
# rate-independent), so it is dropped here.
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
    exponent = 2.0
  []
  [macaulay_n]
    type = MacaulaySplit
    from = 'signed_normal_separation'
    to_positive = 'normal_separation'
    to_negative = 'normal_penetration'
  []
  [mode_mixity]
    type = ModeMixity
  []
  [critical_separation]
    type = CamanhoDavilaCriticalSeparation
    mode_mixity = 'mode_mixity'
    penalty_stiffness = 1e6
    normal_strength = 1e4
    shear_strength = 1e3
  []
  [full_separation]
    type = BenzeggaghKenaneFullSeparation
    mode_mixity = 'mode_mixity'
    critical_separation = 'critical_separation'
    penalty_stiffness = 1e6
    mode_I_fracture_toughness = 1e3
    mode_II_fracture_toughness = 1e2
    shear_strength = 1e3
    eta = 2.2
  []
  [effective_separation]
    type = ScalarPNorm
    from = 'normal_separation tangential_separation'
    to = 'effective_separation'
    exponent = 2.0
  []
  [traction]
    type = BilinearTraction
    traction = 'interface_traction'
    critical_separation = 'critical_separation'
    full_separation = 'full_separation'
    normal_penetration = 'normal_penetration'
    penalty_stiffness = 1e6
  []
  [czm]
    type = ComposedModel
    models = 'decompose tangential_separation macaulay_n effective_separation traction'
    additional_outputs = 'damage'
  []
[]