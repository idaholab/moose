[Models]
  [mandel_stress]
    type = IsotropicMandelStress
    cauchy_stress = 'neml2_stress'
  []
  [overstress]
    type = SR2LinearCombination
    to = 'over_stress'
    from = 'mandel_stress back_stress'
    weights = '1 -1'
  []
  [vonmises]
    type = SR2Invariant
    invariant_type = 'VONMISES'
    tensor = 'over_stress'
    invariant = 'effective_stress'
  []
  [yield]
    type = YieldFunction
    yield_stress = 100
  []
  [flow]
    type = ComposedModel
    models = 'overstress vonmises yield'
  []
  [normality]
    type = Normality
    model = 'flow'
    function = 'yield_function'
    from = 'mandel_stress'
    to = 'flow_direction'
  []
  [flow_rate]
    type = PerzynaPlasticFlowRate
    reference_stress = 500
    exponent = 2
  []
  [Eprate]
    type = AssociativePlasticFlow
  []
  [Xrate]
    # No-static-recovery form of the Chaboche law. Do not switch this to
    # ChabochePlasticHardening, which adds the required A and a recovery terms.
    type = FredrickArmstrongPlasticHardening
    back_stress = 'back_stress'
    C = 1.2e4
    g = 20
  []
  [Erate]
    type = SR2VariableRate
    variable = 'neml2_strain'
  []
  [Eerate]
    type = SR2LinearCombination
    from = 'neml2_strain_rate plastic_strain_rate'
    to = 'elastic_strain_rate'
    weights = '1 -1'
  []
  [elasticity]
    type = LinearIsotropicElasticity
    coefficients = '1e5 0.3'
    coefficient_types = 'YOUNGS_MODULUS POISSONS_RATIO'
    rate_form = true
    strain = 'elastic_strain'
    stress = 'neml2_stress'
  []
  [integrate_stress]
    type = SR2BackwardEulerTimeIntegration
    variable = 'neml2_stress'
  []
  [integrate_X]
    type = SR2BackwardEulerTimeIntegration
    variable = 'back_stress'
  []
  [implicit_rate]
    type = ComposedModel
    models = 'mandel_stress overstress vonmises yield normality flow_rate Eprate Xrate Erate Eerate elasticity integrate_stress integrate_X'
  []
[]

[EquationSystems]
  [eq_sys]
    type = NonlinearSystem
    model = 'implicit_rate'
    unknowns = 'neml2_stress back_stress'
  []
[]

[Solvers]
  [newton]
    type = Newton
    linear_solver = 'lu'
  []
  [lu]
    type = DenseLU
  []
[]

[Models]
  [predictor]
    type = ConstantExtrapolationPredictor
    unknowns_SR2 = 'neml2_stress back_stress'
  []
  [model]
    type = ImplicitUpdate
    equation_system = 'eq_sys'
    solver = 'newton'
    predictor = 'predictor'
  []
[]
