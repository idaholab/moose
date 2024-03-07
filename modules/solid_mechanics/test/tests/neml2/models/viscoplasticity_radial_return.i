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
    type = ElasticStrain
    plastic_strain = 'old_state/internal/Ep'
  []
  [cauchy_stress]
    type = LinearIsotropicElasticity
    youngs_modulus = 1e5
    poisson_ratio = 0.3
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
  [flow]
    type = ComposedModel
    models = 'vonmises yield'
  []
  [normality]
    type = Normality
    model = 'flow'
    function = 'state/internal/fp'
    from = 'state/internal/M'
    to = 'forces/NM'
  []
  [trial_state]
    type = ComposedModel
    models = "trial_elastic_strain cauchy_stress mandel_stress normality"
  []
  ###############################################################################
  # The actual radial return:
  # Since the flow directions are invariant, we only need to integrate
  # the consistency parameter.
  ###############################################################################
  [trial_flow_rate]
    type = ScalarStateRate
    state = 'internal/gamma'
  []
  [plastic_strain_rate]
    type = AssociativePlasticFlow
    flow_direction = 'forces/NM'
  []
  [plastic_strain]
    type = SR2ForwardEulerTimeIntegration
    variable = 'internal/Ep'
  []
  [elastic_strain]
    type = ElasticStrain
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
    variable = 'internal/gamma'
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
