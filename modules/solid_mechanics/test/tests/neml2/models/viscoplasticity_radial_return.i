[Solvers]
  [newton]
    type = Newton
  []
[]

[Models]
  ###############################################################################
  # Use the trial state to precalculate invariant flow directions
  # prior to radial return
  ###############################################################################
  [trial_elastic_strain]
    type = SR2LinearCombination
    from_var = 'forces/E old_state/internal/Ep'
    to_var = 'forces/Ee_trial'
    coefficients = '1 -1'
  []
  [trial_cauchy_stress]
    type = LinearIsotropicElasticity
    coefficients = '1e5 0.3'
    coefficient_types = 'YOUNGS_MODULUS POISSONS_RATIO'
    strain = 'forces/Ee_trial'
    stress = 'forces/S_trial'
  []
  [trial_mandel_stress]
    type = IsotropicMandelStress
    cauchy_stress = 'forces/S_trial'
    mandel_stress = 'forces/M_trial'
  []
  [trial_vonmises]
    type = SR2Invariant
    invariant_type = 'VONMISES'
    tensor = 'forces/M_trial'
    invariant = 'forces/s_trial'
  []
  [trial_yield]
    type = YieldFunction
    yield_stress = 100
    yield_function = 'forces/fp_trial'
    effective_stress = 'forces/s_trial'
  []
  [trial_flow]
    type = ComposedModel
    models = 'trial_vonmises trial_yield'
  []
  [trial_normality]
    type = Normality
    model = 'trial_flow'
    function = 'forces/fp_trial'
    from = 'forces/M_trial'
    to = 'forces/NM'
  []
  [trial_state]
    type = ComposedModel
    models = 'trial_elastic_strain trial_cauchy_stress trial_mandel_stress trial_normality'
  []
  ###############################################################################
  # The actual radial return:
  # Since the flow directions are invariant, we only need to integrate
  # the consistency parameter.
  ###############################################################################
  [trial_flow_rate]
    type = ScalarVariableRate
    variable = 'state/internal/gamma'
  []
  [plastic_strain_rate]
    type = AssociativePlasticFlow
    flow_direction = 'forces/NM'
  []
  [plastic_strain]
    type = SR2ForwardEulerTimeIntegration
    variable = 'state/internal/Ep'
  []
  [elastic_strain]
    type = SR2LinearCombination
    from_var = 'forces/E state/internal/Ep'
    to_var = 'state/internal/Ee'
    coefficients = '1 -1'
  []
  [cauchy_stress]
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
    yield_stress = 100
  []
  [surface]
    type = ComposedModel
    models = "trial_flow_rate
              plastic_strain_rate plastic_strain elastic_strain cauchy_stress mandel_stress
              vonmises yield"
  []
  [flow_rate]
    type = PerzynaPlasticFlowRate
    reference_stress = 1
    exponent = 2
  []
  [integrate_gamma]
    type = ScalarBackwardEulerTimeIntegration
    variable = 'state/internal/gamma'
  []
  [implicit_rate]
    type = ComposedModel
    models = "surface flow_rate integrate_gamma"
  []
  [return_map]
    type = ImplicitUpdate
    implicit_model = 'implicit_rate'
    solver = 'newton'
  []
  [model0]
    type = ComposedModel
    models = "trial_state return_map trial_flow_rate
              plastic_strain_rate plastic_strain"
    additional_outputs = 'state/internal/gamma'
  []
  [model]
    type = ComposedModel
    models = 'model0 elastic_strain cauchy_stress'
    additional_outputs = 'state/internal/Ep'
  []
[]
