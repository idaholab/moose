[Models]
  [adjoint_elasticity_model]
    type = LinearIsotropicElasticity
    youngs_modulus = 5.0
    poisson_ratio = 0.3
    strain = 'forces/E'
  []
  [forward_elasticity_model]
    type = LinearIsotropicElasticity
    youngs_modulus = 5.0
    poisson_ratio = 0.3
    strain = 'forces/E'
  []
[]
