[Tensors]
  [a]
    type = Python
    expr = 'Scalar(1.0)'
  []
  [sdirs]
    type = Python
    expr = 'MillerIndex(torch.tensor([1, 1, 0], dtype=torch.int64))'
  []
  [splanes]
    type = Python
    expr = 'MillerIndex(torch.tensor([1, 1, 1], dtype=torch.int64))'
  []
[]

[Data]
  [crystal_geometry]
    type = CubicCrystal
    lattice_parameter = 'a'
    slip_directions = 'sdirs'
    slip_planes = 'splanes'
  []
[]

[Models]
  # Orientation remains constant as we work with the reference configuration
  [euler_rodrigues]
    type = RotationMatrix
    from = 'orientation'
    to = 'orientation_matrix'
  []
  # Hardening (this is just a very simple hardening model)
  [slip_strength]
    type = SingleSlipStrengthMap
    constant_strength = 50.0
  []
  [voce_hardening]
    type = VoceSingleSlipHardeningRule
    initial_slope = 500.0
    saturated_hardening = 50.0
  []
  # Elasticity: St. Venant-Kirchhoff with Green-Lagrange strain
  [mult_decomp]
    type = R2Multiplication
    A = 'deformation_gradient'
    B = 'plastic_deformation_gradient'
    to = 'elastic_deformation_gradient'
    invert_B = true
  []
  [gl_strain]
    type = GreenLagrangeStrain
    deformation_gradient = 'elastic_deformation_gradient'
    strain = 'elastic_strain'
  []
  [svk]
    type = LinearIsotropicElasticity
    coefficients = '1e5 0.25'
    coefficient_types = 'YOUNGS_MODULUS POISSONS_RATIO'
    strain = 'elastic_strain'
  []
  [elasticity]
    type = ComposedModel
    models = 'mult_decomp gl_strain svk'
  []
  # CP flow rule
  [resolved_shear]
    type = ResolvedShear
  []
  [slip_rule]
    type = PowerLawSlipRule
    n = 2.0
    gamma0 = 2.0e-1
  []
  [sum_slip_rates]
    type = SumSlipRates
  []
  [plastic_velgrad]
    type = PlasticSpatialVelocityGradient
  []
  [plastic_defgrad_rate]
    type = R2Multiplication
    A = 'plastic_spatial_velocity_gradient'
    B = 'plastic_deformation_gradient'
    to = 'plastic_deformation_gradient_rate'
  []
  # Definition of residuals
  [integrate_slip_hardening]
    type = ScalarBackwardEulerTimeIntegration
    variable = 'slip_hardening'
  []
  [integrate_plastic_defgrad]
    type = R2BackwardEulerTimeIntegration
    variable = 'plastic_deformation_gradient'
  []
  # The implicit model that we solve for
  [implicit_rate]
    type = ComposedModel
    models = "euler_rodrigues slip_strength voce_hardening
              elasticity resolved_shear slip_rule sum_slip_rates
              plastic_velgrad plastic_defgrad_rate
              integrate_slip_hardening integrate_plastic_defgrad"
  []
[]

[EquationSystems]
  [eq_sys]
    type = NonlinearSystem
    model = 'implicit_rate'
    unknowns = 'slip_hardening plastic_deformation_gradient'
  []
[]

[Solvers]
  [newton]
    type = NewtonWithLineSearch
    max_linesearch_iterations = 5
    linear_solver = 'lu'
  []
  [lu]
    type = DenseLU
  []
[]

[Models]
  [predictor]
    type = ConstantExtrapolationPredictor
    unknowns_R2 = 'plastic_deformation_gradient'
    unknowns_Scalar = 'slip_hardening'
  []
  [model_without_stress]
    type = ImplicitUpdate
    equation_system = 'eq_sys'
    solver = 'newton'
    predictor = 'predictor'
  []
  # Convert PK2 stress to a full second order tensor
  [full_stress]
    type = SR2ToR2
    input = 'stress'
    output = 'neml2_stress'
  []
  [model]
    type = ComposedModel
    models = 'model_without_stress elasticity full_stress'
    additional_outputs = 'plastic_deformation_gradient'
  []
[]
