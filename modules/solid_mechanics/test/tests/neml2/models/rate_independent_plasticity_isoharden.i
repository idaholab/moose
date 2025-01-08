[Solvers]
  [newton]
    type = Newton
  []
[]

[Models]
  [isoharden]
    type = VoceIsotropicHardening
    saturated_hardening = 100
    saturation_rate = 1.2
  []
  [elastic_strain]
    type = SR2LinearCombination
    from_var = 'forces/E state/internal/Ep'
    to_var = 'state/internal/Ee'
    coefficients = '1 -1'
  []
  [elasticity]
    type = LinearIsotropicElasticity
    coefficients = '1e5 0.3'
    coefficient_types = 'YOUNGS_MODULUS POISSONS_RATIO'
  []
  [mandel_stress]
    type = IsotropicMandelStress
  []
  [vonmises]
    type = SR2Invariant
    invariant_type = 'VONMISES'
    tensor = 'state/internal/M'
    invariant = 'state/internal/s'
  []
  [yield]
    type = YieldFunction
    yield_stress = 5
    isotropic_hardening = 'state/internal/k'
  []
  [flow]
    type = ComposedModel
    models = 'vonmises yield'
  []
  [normality]
    type = Normality
    model = 'flow'
    function = 'state/internal/fp'
    from = 'state/internal/M state/internal/k'
    to = 'state/internal/NM state/internal/Nk'
  []
  [eprate]
    type = AssociativeIsotropicPlasticHardening
  []
  [Eprate]
    type = AssociativePlasticFlow
  []
  [integrate_ep]
    type = ScalarBackwardEulerTimeIntegration
    variable = 'state/internal/ep'
  []
  [integrate_Ep]
    type = SR2BackwardEulerTimeIntegration
    variable = 'state/internal/Ep'
  []
  [consistency]
    type = RateIndependentPlasticFlowConstraint
  []
  [surface]
    type = ComposedModel
    models = "isoharden elastic_strain elasticity
              mandel_stress vonmises
              yield normality eprate Eprate
              consistency integrate_ep integrate_Ep"
  []
  [return_map]
    type = ImplicitUpdate
    implicit_model = 'surface'
    solver = 'newton'
  []
  [model]
    type = ComposedModel
    models = 'return_map elastic_strain elasticity'
    additional_outputs = 'state/internal/Ep state/internal/ep'
  []
[]
