[Models]
  [isoharden]
    type = LinearIsotropicHardening
    hardening_modulus = 1000
  []
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
  [yield_surface]
    type = YieldFunction
    yield_stress = 5
    isotropic_hardening = 'isotropic_hardening'
  []
  [flow]
    type = ComposedModel
    models = 'overstress vonmises yield_surface'
  []
  [normality]
    type = Normality
    model = 'flow'
    function = 'yield_function'
    from = 'mandel_stress isotropic_hardening'
    to = 'flow_direction isotropic_hardening_direction'
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
  [strain_rate]
    type = SR2VariableRate
    variable = 'neml2_strain'
  []
  [elastic_strain_rate]
    type = SR2LinearCombination
    from = 'neml2_strain_rate plastic_strain_rate'
    to = 'elastic_strain_rate'
    weights = '1 -1'
  []
  [elasticity]
    type = LinearIsotropicElasticity
    coefficients = '1e5 0.3'
    coefficient_types = 'YOUNGS_MODULUS POISSONS_RATIO'
    strain = 'elastic_strain'
    stress = 'neml2_stress'
    rate_form = true
  []
  [integrate_ep]
    type = ScalarBackwardEulerTimeIntegration
    variable = 'equivalent_plastic_strain'
  []
  [integrate_X]
    type = SR2BackwardEulerTimeIntegration
    variable = 'back_stress'
  []
  [integrate_S]
    type = SR2BackwardEulerTimeIntegration
    variable = 'neml2_stress'
  []
  [consistency]
    type = FBComplementarity
    a = 'yield_function'
    a_inequality = 'LE'
    b = 'flow_rate'
  []
  [surface]
    type = ComposedModel
    models = 'isoharden mandel_stress overstress vonmises
              yield_surface normality eprate Xrate Eprate
              strain_rate elastic_strain_rate elasticity
              consistency integrate_ep integrate_X integrate_S'
  []
[]

[EquationSystems]
  [eq_sys]
    type = NonlinearSystem
    model = 'surface'
    unknowns = 'equivalent_plastic_strain back_stress neml2_stress flow_rate'
    residuals = 'equivalent_plastic_strain_residual back_stress_residual neml2_stress_residual complementarity'
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
    unknowns_Scalar = 'equivalent_plastic_strain flow_rate'
    unknowns_SR2 = 'back_stress neml2_stress'
  []
  [model]
    type = ImplicitUpdate
    equation_system = 'eq_sys'
    solver = 'newton'
    predictor = 'predictor'
  []
[]
