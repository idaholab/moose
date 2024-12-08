[Models]
  [model1]
    type = LinearIsotropicElasticity
    youngs_modulus = 1
    poisson_ratio = 0.3
    strain = 'forces/E'
  []
  [model2]
    type = LinearIsotropicElasticity
    youngs_modulus = 2
    poisson_ratio = 0.3
    strain = 'forces/E'
  []
[]
