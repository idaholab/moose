[Solvers]
  [newton]
    type = Newton
  []
[]

[Models]
  [isoharden]
    type = LinearIsotropicHardening
    hardening_modulus = 1000
  []
  [mandel_stress]
    type = IsotropicMandelStress
  []
  [overstress]
    type = SR2LinearCombination
    to_var = 'state/internal/O'
    from_var = 'state/internal/M state/internal/X'
    coefficients = '1 -1'
  []
  [vonmises]
    type = SR2Invariant
    invariant_type = 'VONMISES'
    tensor = 'state/internal/O'
    invariant = 'state/internal/s'
  []
  [yield]
    type = YieldFunction
    yield_stress = 5
    isotropic_hardening = 'state/internal/k'
  []
  [flow]
    type = ComposedModel
    models = 'overstress vonmises yield'
  []
  [normality]
    type = Normality
    model = 'flow'
    function = 'state/internal/fp'
    from = 'state/internal/M state/internal/k'
    to = 'state/internal/NM state/internal/Nk'
  []
  [consistency]
    type = RateIndependentPlasticFlowConstraint
  []
  [eprate]
    type = AssociativeIsotropicPlasticHardening
  []
  [Xrate]
    type = FredrickArmstrongPlasticHardening
    C = 1000
    g = 9
  []
  [Eprate]
    type = AssociativePlasticFlow
  []
  [Erate]
    type = SR2VariableRate
    variable = 'forces/E'
    rate = 'forces/E_rate'
  []
  [Eerate]
    type = SR2LinearCombination
    from_var = 'forces/E_rate state/internal/Ep_rate'
    to_var = 'state/internal/Ee_rate'
    coefficients = '1 -1'
  []
  [elasticity]
    type = LinearIsotropicElasticity
    coefficients = '1e5 0.3'
    coefficient_types = 'YOUNGS_MODULUS POISSONS_RATIO'
    rate_form = true
  []
  [integrate_ep]
    type = ScalarBackwardEulerTimeIntegration
    variable = 'state/internal/ep'
  []
  [integrate_X]
    type = SR2BackwardEulerTimeIntegration
    variable = 'state/internal/X'
  []
  [integrate_S]
    type = SR2BackwardEulerTimeIntegration
    variable = 'state/S'
  []
  [surface]
    type = ComposedModel
    models = 'isoharden mandel_stress overstress vonmises
              yield normality eprate Xrate Eprate
              Erate Eerate elasticity
              consistency integrate_ep integrate_X integrate_S'
  []
  [model]
    type = ImplicitUpdate
    implicit_model = 'surface'
    solver = 'newton'
  []
[]
