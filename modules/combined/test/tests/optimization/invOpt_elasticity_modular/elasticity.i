[Models]
  [adjoint_elasticity_model]
    type = LinearIsotropicElasticity
    coefficients = '5.0 0.3'
    coefficient_types = 'YOUNGS_MODULUS POISSONS_RATIO'
    strain = 'adjoint_strain'
    stress = 'adjoint_stress'
  []
  [forward_elasticity_model]
    type = LinearIsotropicElasticity
    coefficients = '5.0 0.3'
    coefficient_types = 'YOUNGS_MODULUS POISSONS_RATIO'
    strain = 'forward_strain'
    stress = 'forward_stress'
  []
[]
