[Models]
  ###############################################################################
  # Use the trial state to precalculate invariant flow directions
  # prior to radial return
  ###############################################################################
  [trial_elastic_strain]
    type = SR2LinearCombination
    from = 'neml2_strain plastic_strain~1'
    to = 'elastic_strain'
    weights = '1 -1'
  []
  [elasticity]
    type = LinearIsotropicElasticity
    coefficients = '1e5 0.3'
    coefficient_types = 'YOUNGS_MODULUS POISSONS_RATIO'
    strain = 'elastic_strain'
    stress = 'neml2_stress'
  []
  [mandel_stress]
    type = IsotropicMandelStress
    cauchy_stress = 'neml2_stress'
  []
  [vonmises]
    type = SR2Invariant
    invariant_type = 'VONMISES'
    tensor = 'mandel_stress'
    invariant = 'effective_stress'
  []
  [yield]
    type = YieldFunction
    yield_stress = 100
  []
  [flow]
    type = ComposedModel
    models = 'vonmises yield'
  []
  [normality]
    type = Normality
    model = 'flow'
    function = 'yield_function'
    from = 'mandel_stress'
    to = 'flow_direction'
  []
  [trial_state]
    type = ComposedModel
    models = 'trial_elastic_strain elasticity mandel_stress normality'
  []
  ###############################################################################
  # The actual radial return:
  # Since the flow directions are invariant, we only need to integrate
  # the consistency parameter.
  ###############################################################################
  [plastic_strain_rate]
    type = AssociativePlasticFlow
    flow_rate = 'gamma_rate'
  []
  [plastic_strain]
    type = SR2ForwardEulerTimeIntegration
    variable = 'plastic_strain'
  []
  [elastic_strain]
    type = SR2LinearCombination
    from = 'neml2_strain plastic_strain'
    to = 'elastic_strain'
    weights = '1 -1'
  []
  [surface]
    type = ComposedModel
    models = 'plastic_strain_rate plastic_strain elastic_strain elasticity mandel_stress vonmises yield'
  []
  [flow_rate]
    type = PerzynaPlasticFlowRate
    reference_stress = 1
    exponent = 2
  []
  [gamma_residual]
    type = ScalarLinearCombination
    from = 'gamma_rate flow_rate'
    to = 'gamma_residual'
    weights = '1 -1'
  []
  [implicit_rate]
    type = ComposedModel
    models = 'surface flow_rate gamma_residual'
  []
[]

[EquationSystems]
  [eq_sys]
    type = NonlinearSystem
    model = 'implicit_rate'
    unknowns = 'gamma_rate'
    residuals = 'gamma_residual'
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
    unknowns_Scalar = 'gamma_rate'
  []
  [return_map]
    type = ImplicitUpdate
    equation_system = 'eq_sys'
    solver = 'newton'
    predictor = 'predictor'
  []
  [model0]
    type = ComposedModel
    models = 'trial_state return_map plastic_strain_rate plastic_strain'
    additional_outputs = 'gamma_rate'
  []
  [model]
    type = ComposedModel
    models = 'model0 elastic_strain elasticity'
    additional_outputs = 'plastic_strain'
  []
[]
