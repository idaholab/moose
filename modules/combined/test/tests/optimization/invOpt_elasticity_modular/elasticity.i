[Models]
  [adjoint_elasticity_model]
    type = LinearIsotropicElasticity
    coefficients = '5.0 0.3'
    coefficient_types = 'YOUNGS_MODULUS POISSONS_RATIO'
    strain = 'forces/E'
  []
  [forward_elasticity_model]
    type = LinearIsotropicElasticity
    coefficients = '5.0 0.3'
    coefficient_types = 'YOUNGS_MODULUS POISSONS_RATIO'
    strain = 'forces/E'
  []
[]
