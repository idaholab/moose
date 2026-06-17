[Models]
  [model]
    type = LinearIsotropicElasticity
    coefficients = '100000 0.3'
    coefficient_types = 'YOUNGS_MODULUS POISSONS_RATIO'
    strain = 'neml2_strain'
    stress = 'neml2_stress'
  []
[]
