[Models]
  [model1]
    type = LinearIsotropicElasticity
    coefficients = '1 0.3'
    coefficient_types = 'YOUNGS_MODULUS POISSONS_RATIO'
    strain = 'forces/E'
  []
  [model2]
    type = LinearIsotropicElasticity
    coefficients = '2 0.3'
    coefficient_types = 'YOUNGS_MODULUS POISSONS_RATIO'
    strain = 'forces/E'
  []
[]
