[Models]
  [model]
    type = LinearIsotropicElasticity
    coefficients = '1 0.3'
    coefficient_types = 'YOUNGS_MODULUS POISSONS_RATIO'
    strain = 'forces/E'
  []
[]
