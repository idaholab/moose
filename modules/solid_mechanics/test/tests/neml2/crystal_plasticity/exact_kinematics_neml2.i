[Tensors]
  [a]
    type = Scalar
    values = '1.0'
  []
  [sdirs]
    type = FillMillerIndex
    values = '1 1 0'
  []
  [splanes]
    type = FillMillerIndex
    values = '1 1 1'
  []
[]

[Solvers]
  [newton]
    type = NewtonWithLineSearch
    max_linesearch_iterations = 5
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
    from = 'forces/r'
    to = 'forces/R'
  []
  # Hardening (this is just a very simple hardening model)
  [slip_strength]
    type = SingleSlipStrengthMap
    constant_strength = 50.0
    slip_hardening = 'state/tauc'
    slip_strengths = 'state/tauc_i'
  []
  [voce_hardening]
    type = VoceSingleSlipHardeningRule
    initial_slope = 500.0
    saturated_hardening = 50.0
    slip_hardening_rate = 'state/tauc_rate'
    slip_hardening = 'state/tauc'
    sum_slip_rates = 'state/gamma_rate'
  []
  # Elasticity: St. Venant-Kirchhoff with Green-Lagrange strain
  [mult_decomp]
    type = R2Multiplication
    A = 'forces/F'
    B = 'state/Fp'
    to = 'state/Fe'
    invert_B = true
  []
  [gl_strain]
    type = GreenLagrangeStrain
    deformation_gradient = 'state/Fe'
    strain = 'state/E'
  []
  [svk]
    type = LinearIsotropicElasticity
    coefficients = '1e5 0.25'
    coefficient_types = 'YOUNGS_MODULUS POISSONS_RATIO'
    strain = 'state/E'
    stress = 'state/S'
  []
  [elasticity]
    type = ComposedModel
    models = 'mult_decomp gl_strain svk'
  []
  # CP flow rule
  [resolved_shear]
    type = ResolvedShear
    resolved_shears = 'state/tau_i'
    stress = 'state/S'
    orientation = 'forces/R'
  []
  [slip_rule]
    type = PowerLawSlipRule
    n = 2.0
    gamma0 = 2.0e-1
    slip_rates = 'state/gamma_rate_i'
    resolved_shears = 'state/tau_i'
    slip_strengths = 'state/tauc_i'
  []
  [sum_slip_rates]
    type = SumSlipRates
    slip_rates = 'state/gamma_rate_i'
    sum_slip_rates = 'state/gamma_rate'
  []
  [plastic_velgrad]
    type = PlasticSpatialVelocityGradient
    plastic_spatial_velocity_gradient = 'state/Lp'
    slip_rates = 'state/gamma_rate_i'
    orientation = 'forces/R'
  []
  [plastic_defgrad_rate]
    type = R2Multiplication
    A = 'state/Lp'
    B = 'state/Fp'
    to = 'state/Fp_rate'
  []
  # Definition of residuals
  [integrate_slip_hardening]
    type = ScalarBackwardEulerTimeIntegration
    variable = 'state/tauc'
  []
  [integrate_plastic_defgrad]
    type = R2BackwardEulerTimeIntegration
    variable = 'state/Fp'
  []
  # The implicit model that we solve for
  [implicit_rate]
    type = ComposedModel
    models = "euler_rodrigues slip_strength voce_hardening
              elasticity resolved_shear slip_rule sum_slip_rates
              plastic_velgrad plastic_defgrad_rate
              integrate_slip_hardening integrate_plastic_defgrad"
  []
  [model_without_stress]
    type = ImplicitUpdate
    implicit_model = 'implicit_rate'
    solver = 'newton'
  []
  # Convert PK2 stress to a full second order tensor
  [full_stress]
    type = SR2toR2
    input = 'state/S'
    output = 'state/full_S'
  []
  [model]
    type = ComposedModel
    models = 'model_without_stress elasticity full_stress'
    additional_outputs = 'state/Fp'
  []
[]
