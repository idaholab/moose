[Models]
  [kinharden]
    type = LinearKinematicHardening
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
  []
  [flow]
    type = ComposedModel
    models = 'overstress vonmises yield_surface'
  []
  [normality]
    type = Normality
    model = 'flow'
    function = 'yield_function'
    from = 'mandel_stress back_stress'
    to = 'flow_direction kinematic_hardening_direction'
  []
  [flow_rate]
    type = PerzynaPlasticFlowRate
    reference_stress = 100
    exponent = 2
  []
  [Kprate]
    type = AssociativeKinematicPlasticHardening
  []
  [Eprate]
    type = AssociativePlasticFlow
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
  [integrate_Kp]
    type = SR2BackwardEulerTimeIntegration
    variable = 'kinematic_plastic_strain'
  []
  [integrate_stress]
    type = SR2BackwardEulerTimeIntegration
    variable = 'neml2_stress'
  []
  [implicit_rate]
    type = ComposedModel
    models = 'kinharden mandel_stress overstress vonmises yield_surface normality flow_rate Eprate Kprate Erate Eerate elasticity integrate_stress integrate_Kp'
  []
[]

[EquationSystems]
  [eq_sys]
    type = NonlinearSystem
    model = 'implicit_rate'
    unknowns = 'neml2_stress kinematic_plastic_strain'
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
    unknowns_SR2 = 'neml2_stress kinematic_plastic_strain'
  []
  [model]
    type = ImplicitUpdate
    equation_system = 'eq_sys'
    solver = 'newton'
    predictor = 'predictor'
  []
[]
