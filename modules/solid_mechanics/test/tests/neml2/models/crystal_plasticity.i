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
    lattice_parameter = "a"
    slip_directions = "sdirs"
    slip_planes = "splanes"
  []
[]

[Models]
  [spatial_velocity_gradient]
    type = R2IncrementToRate
    variable = 'forces/spatial_velocity_increment'
    time = 'forces/t'
    rate = 'forces/spatial_velocity_gradient'
  []
  [split_to_deformation_rate]
    type = R2toSR2
    input = 'forces/spatial_velocity_gradient'
    output = 'forces/deformation_rate'
  []
  [split_to_vorticity]
    type = R2toWR2
    input = 'forces/spatial_velocity_gradient'
    output = 'forces/vorticity'
  []
  [euler_rodrigues]
    type = RotationMatrix
    from = 'state/orientation'
    to = 'state/orientation_matrix'
  []
  [elasticity]
    type = LinearIsotropicElasticity
    coefficient_types = 'YOUNGS_MODULUS POISSONS_RATIO'
    coefficients = '1e5 0.25'
    strain = "state/elastic_strain"
    stress = "state/internal/cauchy_stress"
  []
  [resolved_shear]
    type = ResolvedShear
  []
  [elastic_stretch]
    type = ElasticStrainRate
  []
  [plastic_spin]
    type = PlasticVorticity
  []
  [plastic_deformation_rate]
    type = PlasticDeformationRate
  []
  [orientation_rate]
    type = OrientationRate
  []
  [sum_slip_rates]
    type = SumSlipRates
  []
  [slip_rule]
    type = PowerLawSlipRule
    n = 8.0
    gamma0 = 2.0e-1
  []
  [slip_strength]
    type = SingleSlipStrengthMap
    constant_strength = 50.0
  []
  [voce_hardening]
    type = VoceSingleSlipHardeningRule
    initial_slope = 500.0
    saturated_hardening = 50.0
  []
  [integrate_slip_hardening]
    type = ScalarBackwardEulerTimeIntegration
    variable = 'state/internal/slip_hardening'
  []
  [integrate_elastic_strain]
    type = SR2BackwardEulerTimeIntegration
    variable = 'state/elastic_strain'
  []
  [integrate_orientation]
    type = WR2ImplicitExponentialTimeIntegration
    variable = 'state/orientation'
  []

  [implicit_rate]
    type = ComposedModel
    models = "spatial_velocity_gradient split_to_deformation_rate split_to_vorticity euler_rodrigues elasticity orientation_rate resolved_shear
              elastic_stretch plastic_deformation_rate plastic_spin
              sum_slip_rates slip_rule slip_strength voce_hardening
              integrate_slip_hardening integrate_elastic_strain integrate_orientation"
  []
  [model_without_stress]
    type = ImplicitUpdate
    implicit_model = 'implicit_rate'
    solver = 'newton'
  []
  [full_stress]
    type = SR2toR2
    input = 'state/internal/cauchy_stress'
    output = 'state/internal/full_cauchy_stress'
  []
  [model]
    type = ComposedModel
    models = 'model_without_stress elasticity full_stress'
    additional_outputs = 'state/elastic_strain'
  []
[]
